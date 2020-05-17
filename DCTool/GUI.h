#pragma once

#include "./imgui-1.76/imgui.h"
#include "./imgui-1.76/imgui_impl_win32.h"
#include "./imgui-1.76/imgui_impl_dx11.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define ERROR_COLOR {1.0f , 0.0f, 0.0f, 1.0f}
#define SUCCESS_COLOR {0.0f , 1.0f, 0.0f, 1.0f}
#define INFO_COLOR {1.0f , 1.0f, 0.0f, 1.0f}
#define INACTIVE_TAB_COLOR ImVec4{1.0f , 1.0f, 1.0f, 1.0f}
#define ACTIVE_TAB_COLOR ImVec4{1.0f , 1.0f, 0.0f, 1.0f}

#define TAB_COLOR(CurrentTab, Tab) (CurrentTab == Tab ? ACTIVE_TAB_COLOR : INACTIVE_TAB_COLOR)

enum class UIElementType {
	Toolbar,
	Scene,
	Settings,
	DC,

	MAX
};

struct NOVTABLE UIRendered {

	UIRendered(UIElementType Type) :UIType(Type) {}

	void Close()noexcept {
		bDisplay = false;
	}
	void Show()noexcept {
		bDisplay = true;
	}
	bool IsDisplayed() const noexcept { return bDisplay; }
	virtual void RenderUI() = 0;

protected:
	inline void RenderCloseButton() noexcept {
		if (ImGui::Button("Close")) {
			bDisplay = false;
		}
	}

	bool	bShow = true;
	bool	bDisplay = true;
	const	UIElementType UIType;
private:
	void Render() {
		if (!bDisplay) {
			return;
		}

		RenderUI();
	}

	friend struct GuiManager;
};

template<UIElementType UIEType>
struct NOVTABLE UIElement : UIRendered {

	typedef UIElement<UIEType> Base;

	struct Traits {
		enum { Type = UIEType };
	};

	UIElement() : UIRendered(UIEType) {}

	virtual bool Initialize();

	float		Width = 0.0f;
	float		Height = 0.0f;
};

//Main Elements
struct Toolbar : UIElement<UIElementType::Toolbar> {
	void RenderUI() override;
};

struct GuiManager  {
	int Initialize(int width, int height);
	void Update(float dt);
	void Render(float dt);
	void Shutdown();

	void EnableKeyboard() {
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	}
	void StyleDark() {
		ImGui::StyleColorsDark();
	}
	void StyleClassic() {
		ImGui::StyleColorsClassic();
	}

	template<UIElementType UIEType>
	bool RegisterUI(UIElement<UIEType>* Element) noexcept {
		UIEmlements[(int)UIEType] = Element;

		return true;
	}

	template<UIElementType UIEType>
	bool UnregisterUI(UIElement<UIEType>* Element) noexcept {
		UIEmlements[(int)UIEType] = nullptr;
		return true;
	}

	template<UIElementType UIEType>
	TRef<UIRendered> GetElement() noexcept {
		return UIEmlements[(int)UIEType];
	}

	template<typename UIElement>
	TRef<UIElement> GetElementAs() noexcept {
		return reinterpret_cast<UIElement*>(UIEmlements[(int)UIElement::Traits::Type].Get());
	}

	template<UIElementType UIEType>
	bool TryShowElement() noexcept {
		if (!UIEmlements[(int)UIEType].Get()) {
			return false;
		}

		UIEmlements[(int)UIEType]->Show();

		return true;
	}

	template<UIElementType UIEType>
	bool TryCloseElement() noexcept {
		if (!UIEmlements[(int)UIEType].Get()) {
			return false;
		}

		UIEmlements[(int)UIEType]->Close();

		return true;
	}

private:
	static ULONG				IdPool;

	TRef<ID3D10Texture2D>		ImGuiSurface = nullptr;
	TRef<ImGuiContext>			ImGuiContext = nullptr;

	TRef<UIRendered>			UIEmlements[(int)UIElementType::MAX];

	//Main Elements
	Toolbar						Toolbar;
};

extern GuiManager		GGUIManager;

template<UIElementType UIEType>
inline bool UIElement<UIEType>::Initialize()
{
	if (!GGUIManager.RegisterUI<UIEType>(this)) {
		return false;
	}

	return true;
}
