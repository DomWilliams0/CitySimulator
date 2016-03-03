#ifndef CITYSIM_TEST_UTILS_HPP
#define CITYSIM_TEST_UTILS_HPP
#include "gtest/gtest.h"
#include "services.hpp"

#define EXPECT_ERROR_MESSAGE(block, errorMessage) \
                    try\
                    {\
                        block\
                        FAIL() << "Expected std::runtime_error";\
                    } catch (const std::runtime_error &e)\
                    {\
                        if (errorMessage != "")\
                            EXPECT_EQ(e.what(), std::string(errorMessage));\
                    } catch (...)\
                    {\
                        FAIL() << "Expected std::runtime_error";\
                    }\



#endif

#define DATA_ROOT "data"