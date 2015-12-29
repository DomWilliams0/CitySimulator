#ifndef CITYSIMULATOR_TEST_UTILS_H
#define CITYSIMULATOR_TEST_UTILS_H
#include "gtest/gtest.h"

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
