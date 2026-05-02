find_package(ICU COMPONENTS data i18n io uc)

if(ICU_FOUND)
    add_library(icu INTERFACE)
    target_link_libraries(icu INTERFACE ICU::data ICU::i18n ICU::io ICU::uc)
endif()
