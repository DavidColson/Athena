// Copyright 2020-2021 David Colson. All rights reserved.

#include "AABB.h"

#include "TypeSystem/TypeDatabase.h"

REFLECT_TEMPLATED_BEGIN(AABBf)
REFLECT_MEMBER(min)
REFLECT_MEMBER(max)
REFLECT_END()


REFLECT_TEMPLATED_BEGIN(AABBd)
REFLECT_MEMBER(min)
REFLECT_MEMBER(max)
REFLECT_END()