#include "MainWindow.h"

#include "AnimatedSoundOptionsDialog.h"
#include "WWAudio.h"
#include "assetmgr.h"
#include "wwmath.h"

#include <QApplication>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QPointer>
#include <QStringList>
#include <QTimer>

#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

QStringList CollectStartupFiles(const QStringList &arguments)
{
    QStringList files;
    for (int index = 1; index < arguments.size(); ++index) {
        const QFileInfo info(arguments[index]);
        if (info.suffix().compare("w3d", Qt::CaseInsensitive) == 0) {
            files.append(info.absoluteFilePath());
        }
    }
    return files;
}

#ifdef _WIN32

class WindowsSingleInstance final
{
public:
    WindowsSingleInstance()
    {
        _mutex = ::CreateMutexW(nullptr, FALSE, MutexName());
        if (_mutex && ::GetLastError() == ERROR_ALREADY_EXISTS) {
            _primary = false;
            return;
        }

        _primary = true;
        createMessageWindow();
    }

    ~WindowsSingleInstance()
    {
        if (_messageWindow) {
            ::DestroyWindow(_messageWindow);
        }
        if (_windowClass) {
            ::UnregisterClassW(WindowClassName(), ::GetModuleHandleW(nullptr));
        }
        if (_mutex) {
            ::CloseHandle(_mutex);
        }
    }

    bool isPrimary() const
    {
        return _primary;
    }

    bool forwardToPrimary(const QStringList &files) const
    {
        HWND receiver = nullptr;
        for (int attempt = 0; attempt < 100 && !receiver; ++attempt) {
            receiver = ::FindWindowExW(HWND_MESSAGE, nullptr, WindowClassName(), nullptr);
            if (!receiver) {
                ::Sleep(50);
            }
        }
        if (!receiver) {
            return false;
        }

        DWORD primary_process_id = 0;
        ::GetWindowThreadProcessId(receiver, &primary_process_id);
        if (primary_process_id != 0) {
            ::AllowSetForegroundWindow(primary_process_id);
        }

        const QString payload = files.join(QLatin1Char('\n'));
        COPYDATASTRUCT copy_data = {};
        copy_data.dwData = CopyDataId();
        copy_data.cbData = static_cast<DWORD>((payload.size() + 1) * sizeof(wchar_t));
        copy_data.lpData = const_cast<ushort *>(payload.utf16());

        DWORD_PTR response = 0;
        const LRESULT sent = ::SendMessageTimeoutW(
            receiver,
            WM_COPYDATA,
            0,
            reinterpret_cast<LPARAM>(&copy_data),
            SMTO_ABORTIFHUNG | SMTO_BLOCK,
            5000,
            &response);
        return sent != 0 && response != 0;
    }

    void attach(W3DViewMainWindow *window)
    {
        _window = window;
        if (!_activationPending && _pendingFiles.isEmpty()) {
            return;
        }

        const QStringList files = _pendingFiles;
        _pendingFiles.clear();
        _activationPending = false;
        activateAndOpen(files);
    }

private:
    static const wchar_t *MutexName()
    {
        return L"Local\\OpenW3D.W3DViewQt.SingleInstance.v1";
    }

    static const wchar_t *WindowClassName()
    {
        return L"OpenW3D.W3DViewQt.SingleInstance.MessageWindow.v1";
    }

    static ULONG_PTR CopyDataId()
    {
        return static_cast<ULONG_PTR>(0x57334451U); // "W3DQ"
    }

    void createMessageWindow()
    {
        WNDCLASSEXW window_class = {};
        window_class.cbSize = sizeof(window_class);
        window_class.lpfnWndProc = &WindowsSingleInstance::WindowProc;
        window_class.hInstance = ::GetModuleHandleW(nullptr);
        window_class.lpszClassName = WindowClassName();
        _windowClass = ::RegisterClassExW(&window_class);
        if (!_windowClass) {
            return;
        }

        _messageWindow = ::CreateWindowExW(
            0,
            WindowClassName(),
            L"",
            0,
            0,
            0,
            0,
            0,
            HWND_MESSAGE,
            nullptr,
            window_class.hInstance,
            this);
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        auto *instance = reinterpret_cast<WindowsSingleInstance *>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            const auto *create = reinterpret_cast<const CREATESTRUCTW *>(lparam);
            instance = static_cast<WindowsSingleInstance *>(create->lpCreateParams);
            ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance));
        }

        if (message == WM_COPYDATA && instance) {
            const auto *copy_data = reinterpret_cast<const COPYDATASTRUCT *>(lparam);
            return instance->receiveCopyData(copy_data) ? TRUE : FALSE;
        }

        if (message == WM_NCDESTROY) {
            ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        }
        return ::DefWindowProcW(hwnd, message, wparam, lparam);
    }

    bool receiveCopyData(const COPYDATASTRUCT *copy_data)
    {
        if (!copy_data || copy_data->dwData != CopyDataId() || !copy_data->lpData ||
            copy_data->cbData < sizeof(wchar_t) ||
            (copy_data->cbData % sizeof(wchar_t)) != 0) {
            return false;
        }

        const auto *text = static_cast<const wchar_t *>(copy_data->lpData);
        const qsizetype character_count =
            static_cast<qsizetype>(copy_data->cbData / sizeof(wchar_t));
        if (text[character_count - 1] != L'\0') {
            return false;
        }

        const QString payload = QString::fromWCharArray(text, character_count - 1);
        const QStringList files = payload.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        activateAndOpen(files);
        return true;
    }

    void activateAndOpen(const QStringList &files)
    {
        if (!_window) {
            _activationPending = true;
            _pendingFiles.append(files);
            return;
        }

        if (_window->isMinimized()) {
            _window->showNormal();
        } else {
            _window->show();
        }
        const HWND hwnd = reinterpret_cast<HWND>(_window->winId());
        ::BringWindowToTop(hwnd);
        ::SetForegroundWindow(hwnd);
        _window->raise();
        _window->activateWindow();

        if (files.isEmpty()) {
            return;
        }

        const QPointer<W3DViewMainWindow> window(_window);
        QTimer::singleShot(0, _window, [window, files]() {
            if (!window) {
                return;
            }
            for (const QString &path : files) {
                const QFileInfo info(path);
                if (info.suffix().compare("w3d", Qt::CaseInsensitive) == 0) {
                    window->openFilePath(info.absoluteFilePath());
                }
            }
        });
    }

    HANDLE _mutex = nullptr;
    ATOM _windowClass = 0;
    HWND _messageWindow = nullptr;
    QPointer<W3DViewMainWindow> _window;
    QStringList _pendingFiles;
    bool _primary = true;
    bool _activationPending = false;
};

#endif

} // namespace

int main(int argc, char *argv[])
{
    int result = 0;
    {
        QApplication app(argc, argv);
        QCoreApplication::setOrganizationName("OpenW3D");
        QCoreApplication::setApplicationName("W3DViewQt");
        QApplication::setWindowIcon(QIcon(":/w3dview/app.ico"));

        const QStringList startup_files = CollectStartupFiles(app.arguments());

#ifdef _WIN32
        WindowsSingleInstance single_instance;
        if (!single_instance.isPrimary()) {
            if (single_instance.forwardToPrimary(startup_files)) {
                return EXIT_SUCCESS;
            }
            QMessageBox::critical(nullptr,
                                  "W3DViewQt",
                                  "Unable to contact the running W3DViewQt instance.");
            return EXIT_FAILURE;
        }
#endif

        WWMath::Init();
        AnimatedSoundOptionsDialog::LoadAnimatedSoundSettings();

        {
            WW3DAssetManager asset_manager;
            asset_manager.Set_WW3D_Load_On_Demand(true);
            asset_manager.Set_Activate_Fog_On_Load(true);

            WWAudioClass *audio_mgr = WWAudioClass::Create_Instance();
            if (audio_mgr) {
                audio_mgr->Initialize();
            }

            {
                W3DViewMainWindow window;
                window.show();

#ifdef _WIN32
                single_instance.attach(&window);
#endif
                if (!startup_files.isEmpty()) {
                    QTimer::singleShot(0, &window, [&window, startup_files]() {
                        for (const QString &path : startup_files) {
                            window.openFilePath(path);
                        }
                    });
                }

                result = app.exec();
            }

            if (audio_mgr) {
                delete audio_mgr;
                audio_mgr = nullptr;
            }
        }
    }

    WWMath::Shutdown();
    return result;
}
