#include <cstdint>

class TestApp
{
public:
	TestApp();
	~TestApp();

	void Resize(uint32_t width, uint32_t height);
	void Init();
	void Step();
};