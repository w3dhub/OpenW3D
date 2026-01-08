find_package(ICU REQUIRED data i18n io uc)

add_library(icu INTERFACE)
target_link_libraries(icu INTERFACE ICU::data ICU::i18n ICU::io ICU::uc)
