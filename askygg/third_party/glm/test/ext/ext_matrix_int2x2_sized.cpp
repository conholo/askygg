#include <glm/ext/matrix_int2x2_sized.hpp>

#if GLM_HAS_STATIC_assert
	static_assert(sizeof(glm::i8mat2x2) == 4, "int8 size isn't 1 byte on this platform");
	static_assert(sizeof(glm::i16mat2x2) == 8, "int16 size isn't 2 bytes on this platform");
	static_assert(sizeof(glm::i32mat2x2) == 16, "int32 size isn't 4 bytes on this platform");
	static_assert(sizeof(glm::i64mat2x2) == 32, "int64 size isn't 8 bytes on this platform");
#endif

static int test_comp()
{
	int Error = 0;

	Error += sizeof(glm::i8mat2x2) < sizeof(glm::i16mat2x2) ? 0 : 1;
	Error += sizeof(glm::i16mat2x2) < sizeof(glm::i32mat2x2) ? 0 : 1;
	Error += sizeof(glm::i32mat2x2) < sizeof(glm::i64mat2x2) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_comp();

	return Error;
}
