#pragma once
#include <cstdint>
#include "Frame.h"
#include <functional>
#include <memory>

namespace modbus {
	template <typename T>
	using Getter = std::function<T(uint16_t)>;
	template <typename T>
	using Setter = std::function<void(uint16_t, T value)>;

	// default stuff
	bool defaultBoolGetter(uint16_t addr);
	uint16_t defaultShortGetter(uint16_t addr);
	void defaultBoolSetter(uint16_t a, bool b);
	void defaultShortSetter(uint16_t a, uint16_t s);

	using SenderStream = std::function<void(const uint8_t* msg, int len)>;
	struct Accessors {
		Getter<bool> getBool = defaultBoolGetter;
		Setter<bool> setBool = defaultBoolSetter;
		Getter<uint16_t> getShort = defaultShortGetter;
		Setter<uint16_t> setShort = defaultShortSetter;
	};

	enum ModbusException {
		ILLEGAL_FUNC = 1,
		ILLEGAL_DATA_ADDR = 2,
		ILLEGAL_DATA_VALUE = 3,
		SLAVE_DEVICE_FAILIURE = 4,
		ACKNOWLEDGE = 5,
		SLAVE_DEVICE_BUSY = 6,
		NEGATIVE_ACKNOWLEDGE = 7
	};

	class Responder {
	public:
		Frame frame;
		Responder(Frame frame, SenderStream send): frame(std::move(frame)), send(std::move(send)) {};
		void respWithAccessors(Accessors &accessors);
		void respWithException(ModbusException exception);
		void respWithRaw(uint8_t *msg, int len);
	private:
		SenderStream send;
	};

	using ModbusCallback = std::function<void(Responder)>;
}

