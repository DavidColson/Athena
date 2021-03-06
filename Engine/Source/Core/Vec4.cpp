// Copyright 2020-2021 David Colson. All rights reserved.

#include "Vec4.h"

#include "TypeSystem/TypeDatabase.h"

REFLECT_TEMPLATED_BEGIN(Vec4f)
REFLECT_MEMBER(x)
REFLECT_MEMBER(y)
REFLECT_MEMBER(z)
REFLECT_MEMBER(w)
REFLECT_END()

REFLECT_TEMPLATED_BEGIN(Vec4d)
REFLECT_MEMBER(x)
REFLECT_MEMBER(y)
REFLECT_MEMBER(z)
REFLECT_MEMBER(w)
REFLECT_END()