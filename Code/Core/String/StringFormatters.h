#pragma once
#include "External/fmt/format.h"

// FMT formatters

namespace fmt {
	template <>
	struct formatter<StringView> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const StringView& p, FormatContext& ctx) {
			return formatter<std::wstring_view>::format(std::wstring_view(p.GetData(), static_cast<size_t>(p.GetLength())), ctx);
		}
	};
}

namespace fmt {
	template <>
	struct formatter<StringSlice> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const StringSlice& p, FormatContext& ctx) {
			return formatter<std::wstring_view>::format(std::wstring_view(p.GetData(), static_cast<size_t>(p.GetLength())), ctx);
		}
	};
}

namespace fmt {
	template <>
	struct formatter<String> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const String& p, FormatContext& ctx) {
			return formatter<std::wstring_view>::format(std::wstring_view(p.GetData(), static_cast<size_t>(p.GetLength())), ctx);
		}
	};
}
