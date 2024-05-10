#pragma once

struct ImFont;

namespace Render
{
	enum class FontType
	{
		Regular,
		Bold,
		Italic,
		Large,
		Title,
		Count
	};

	struct FontMap
	{
		FontMap()
		{
			Clear();
		}
		void Clear()
		{
			for (size_t i = 0; i < (size_t)FontType::Count; ++i)
				myFonts[i] = nullptr;
		}
		void SetFont(FontType aFontType, ImFont* aFont) { myFonts[(size_t)aFontType] = aFont; }
		ImFont* GetFont(FontType aFontType) const { return myFonts[(size_t)aFontType]; }
	private:
		std::array<ImFont*, (size_t)FontType::Count> myFonts;
	};
}
