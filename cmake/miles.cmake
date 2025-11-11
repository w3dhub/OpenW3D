FetchContent_Declare(
    miles
    GIT_REPOSITORY https://github.com/TheSuperHackers/miles-sdk-stub.git
    GIT_TAG        ff364dd3308a7c3470188427cfa481fe7d993551
)

set (MILES_NOFLOAT TRUE)
FetchContent_MakeAvailable(miles)
