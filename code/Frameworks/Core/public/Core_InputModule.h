#pragma once
#include "Core_Module.h"
#include "Core_SlotArray.h"

#include <functional>

struct GLFWwindow;

namespace Input
{
	enum MouseButton
	{
		MouseLeft,
		MouseRight,
		MouseMiddle,

		MouseCount,
	};

	enum Key
	{
		Key0,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,

		KeyA,
		KeyB,
		KeyC,
		KeyD,
		KeyE,
		KeyF,
		KeyG,
		KeyH,
		KeyI,
		KeyJ,
		KeyK,
		KeyL,
		KeyM,
		KeyN,
		KeyO,
		KeyP,
		KeyQ,
		KeyR,
		KeyS,
		KeyT,
		KeyU,
		KeyV,
		KeyW,
		KeyX,
		KeyY,
		KeyZ,

		KeySpace,
		KeyApostrophe,
		KeyGraveAccent,
		KeyComma,
		KeySemicolon,
		KeyPeriod,
		KeyHyphen,
		KeyEqual,
		KeySlash,
		KeyBackSlash,
		KeyLeftBracket,
		KeyRightBracket,

		KeyEscape,
		KeyEnter,
		KeyTab,
		KeyBackspace,
		KeyInsert,
		KeyDelete,
		KeyRight,
		KeyLeft,
		KeyDown,
		KeyUp,
		KeyPageUp,
		KeyPageDown,
		KeyHome,
		KeyEnd,
		KeyCapsLock,
		KeyScrollLock,
		KeyNumLock,
		KeyPrintScreen,
		KeyPause,
		KeyF1,
		KeyF2,
		KeyF3,
		KeyF4,
		KeyF5,
		KeyF6,
		KeyF7,
		KeyF8,
		KeyF9,
		KeyF10,
		KeyF11,
		KeyF12,
		KeyLeftShift,
		KeyLeftCtrl,
		KeyLeftAlt,
		KeyLeftSuper,
		KeyRightShift,
		KeyRightCtrl,
		KeyRightAlt,
		KeyRightSuper,
		KeyMenu,

		KeyNumPad0,
		KeyNumPad1,
		KeyNumPad2,
		KeyNumPad3,
		KeyNumPad4,
		KeyNumPad5,
		KeyNumPad6,
		KeyNumPad7,
		KeyNumPad8,
		KeyNumPad9,
		KeyNumPadDecimal,
		KeyNumPadDivide,
		KeyNumPadMultiply,
		KeyNumPadSubtract,
		KeyNumPadAdd,
		KeyNumPadEnter,
		KeyNumPadEqual,

		KeyCount,
	};

	enum class Status
	{
		Released,
		Pressed,
		Repeated,
		Unknown,
	};

	enum Modifier
	{
		ModNone = 0x00,
		ModShift = 0x01,
		ModControl = 0x02,
		ModAlt = 0x04,
		ModSuper = 0x08,
		ModCapsLock = 0x10,
		ModNumLock = 0x20,
	};

	typedef std::function<void(MouseButton, Status, Modifier)> MouseCallback;
	typedef std::function<void(double, double)> CursorCallback;
	typedef std::function<void(uint64, double, double, bool)> TouchCallback;
	typedef std::function<void(Key, Status, Modifier)> KeyCallback;
	typedef std::function<void(double, double)> ScrollCallback;
	typedef std::function<void(uint)> CharacterCallback;

	template<typename CallbackType>
	struct CallbackEntry
	{
		void Clear() { myWindow = nullptr; myCallback = nullptr; }
		bool IsSet() const { return myCallback != nullptr; }
		GLFWwindow* myWindow = nullptr;
		CallbackType myCallback = nullptr;
	};
}

namespace Core
{
	class WindowManager;

	class InputModule : public Module
	{
	DECLARE_CORE_MODULE(InputModule, "Input")

	protected:
		void OnRegister() override;

	public:
		Input::Status PollMouseInput(Input::MouseButton aButton, GLFWwindow* aWindow) const;
		Input::Status PollKeyInput(Input::Key aKey, GLFWwindow* aWindow) const;
		void PollCursorPosition(double& anOutX, double& anOutY, GLFWwindow* aWindow) const;

		uint AddMouseCallback(Input::MouseCallback aCallback, GLFWwindow* aWindow);
		void RemoveMouseCallback(uint aCallbakId);

		uint AddCursorCallback(Input::CursorCallback aCallback, GLFWwindow* aWindow);
		void RemoveCursorCallback(uint aCallbakId);

		uint AddTouchCallback(Input::TouchCallback aCallback, GLFWwindow* aWindow);
		void RemoveTouchCallback(uint aCallbakId);

		uint AddKeyCallback(Input::KeyCallback aCallback, GLFWwindow* aWindow);
		void RemoveKeyCallback(uint aCallbakId);

		uint AddScrollCallback(Input::ScrollCallback aCallback, GLFWwindow* aWindow);
		void RemoveScrollCallback(uint aCallbakId);

		uint AddCharacterCallback(Input::CharacterCallback aCallback, GLFWwindow* aWindow);
		void RemoveCharacterCallback(uint aCallbakId);

#if DEBUG_BUILD
		// Used by the debuggers to fake touches
		uint64 FakeTouchBegin(GLFWwindow* aWindow, double anX, double anY);
		void FakeTouchMove(GLFWwindow* aWindow, uint64 aFakeFingerId, double anX, double anY);
		void FakeTouchEnd(GLFWwindow* aWindow, uint64 aFakeFingerId);
#endif

	protected:
		friend class WindowModule;
		static void OnMouseCallback(GLFWwindow* aWindow, int aButton, int anAction, int someMods);
		static void OnCursorCallback(GLFWwindow* aWindow, double anX, double anY);
		static void OnTouchCallback(GLFWwindow* aWindow, uint64 aFingerId, double anX, double anY, int anUp);
		static void OnKeyCallback(GLFWwindow* aWindow, int aKey, int aScanCode, int anAction, int someMods);
		static void OnScrollCallback(GLFWwindow* aWindow, double anX, double anY);
		static void OnCharacterCallback(GLFWwindow* aWindow, uint aUnicodeCodePoint);

	private:
		SlotArray<Input::CallbackEntry<Input::MouseCallback>> myMouseCallbacks;
		SlotArray<Input::CallbackEntry<Input::CursorCallback>> myCursorCallbacks;
		SlotArray<Input::CallbackEntry<Input::TouchCallback>> myTouchCallbacks;
		SlotArray<Input::CallbackEntry<Input::KeyCallback>> myKeyCallbacks;
		SlotArray<Input::CallbackEntry<Input::ScrollCallback>> myScrollCallbacks;
		SlotArray<Input::CallbackEntry<Input::CharacterCallback>> myCharacterCallbacks;

#if DEBUG_BUILD
		uint64 GetNextFakeTouchId() const;
		std::set<uint64> myFakeTouchIds;
#endif
	};
} // namespace Input
