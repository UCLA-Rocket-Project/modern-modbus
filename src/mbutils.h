#pragma once
#include <endian.h>
#include <cstdint>
namespace modbus {
	inline uint16_t mostSignificantHalf(float f) {
		uint32_t tmp = *(uint32_t*)&f;
		tmp = htobe32(tmp);
		uint16_t *ptr = (uint16_t*)&tmp;
		return ptr[0];
	}
	inline uint16_t leastSignificantHalf(float f) {
		uint32_t tmp = *(uint32_t*)&f;
		tmp = htobe32(tmp);
		uint16_t *ptr = (uint16_t*)&tmp;
		return ptr[1];
	}
	inline uint16_t mostSignificantHalf(uint32_t l) {
		l = htobe32(l);
		uint16_t *ptr = (uint16_t*)&l;
		return ptr[0];
	}
	inline uint16_t leastSignificantHalf(uint32_t l) {
		l = htobe32(l);
		uint16_t *ptr = (uint16_t*)&l;
		return ptr[1];
	}
}