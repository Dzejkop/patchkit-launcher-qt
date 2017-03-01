#ifndef CUSTOMMACROS_H
#define CUSTOMMACROS_H

#include "catch.h"

#define EXPECT(operation, exception) \
    { \
        bool check = false; \
        try \
        { \
            operation; \
        } \
        catch(exception) \
        { \
            check = true; \
        } \
        REQUIRE(check == true); \
    }

#define EXPECT_ANY(operation) EXPECT(operation, ...)

#endif // CUSTOMMACROS_H
