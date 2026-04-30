$action = New-ScheduledTaskAction -Execute "cmd.exe" -Argument "/c cd /d E:\OpenW3D\build-both && render_compare.exe bgfx_local.bmp > output_local.log 2>&1"
$principal = New-ScheduledTaskPrincipal -UserId "dadud" -LogonType Interactive -RunLevel Highest
$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -ExecutionTimeLimit (New-TimeSpan -Minutes 5)
Register-ScheduledTask -TaskName "BGFXTest3" -Action $action -Principal $principal -Settings $settings -Force
Write-Host "Task created successfully"