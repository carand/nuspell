/* Copyright 2018 Dimitrij Mijoski
 *
 * This file is part of Nuspell.
 *
 * Nuspell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nuspell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Nuspell.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NUSPELL_STRUCTURES_HXX
#define NUSPELL_STRUCTURES_HXX

#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/range/iterator_range_core.hpp>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>

namespace nuspell {

/**
 * @brief A Set class backed by a string. Very useful for small sets.
 */
class Flag_Set {
	std::u16string flags;
	auto sort_uniq() -> void;

      public:
	// creation
	Flag_Set() = default;
	Flag_Set(const std::u16string& s);
	Flag_Set(std::u16string&& s);
	auto operator=(const std::u16string& s) -> Flag_Set&;
	auto operator=(std::u16string&& s) -> Flag_Set&;

	// insert
	// auto insert(char16_t flag) -> It;

	// bulk insert
	// auto insert(It first, It last) -> void;
	auto insert(const std::u16string& s) -> void;
	auto operator+=(const std::u16string& s) -> Flag_Set&
	{
		insert(s);
		return *this;
	}

	// erase
	auto erase(char16_t flag) -> bool;

	// bulk erase
	// auto clear() { return flags.clear(); }

	// access
	auto size() const noexcept { return flags.size(); }
	auto data() const noexcept -> const std::u16string& { return flags; }
	operator const std::u16string&() const noexcept { return flags; }
	auto empty() const noexcept -> bool { return flags.empty(); }
	auto exists(char16_t flag) const -> bool
	{
		// This method is most commonly called (on hotpath)
		// put it in header so it is inlined.
		// Flags are short strings.
		// Optimized linear search should be better for short strings.
		return flags.find(flag) != flags.npos;
	}
	auto count(char16_t flag) const -> size_t { return exists(flag); }

	auto begin() const noexcept { return flags.begin(); }
	auto end() const noexcept { return flags.end(); }
	auto friend swap(Flag_Set& a, Flag_Set& b) { a.flags.swap(b.flags); }
};

template <class CharT>
class Substr_Replacer {
      public:
	using StrT = std::basic_string<CharT>;
	using Table_Pairs = std::vector<std::pair<StrT, StrT>>;

      private:
	Table_Pairs table;
	void sort_uniq();

      public:
	Substr_Replacer() = default;
	Substr_Replacer(const Table_Pairs& v);
	Substr_Replacer(Table_Pairs&& v);

	auto operator=(const Table_Pairs& v) -> Substr_Replacer&;
	auto operator=(Table_Pairs&& v) -> Substr_Replacer&;
	template <class Range>
	auto operator=(const Range& range) -> Substr_Replacer&
	{
		table.assign(std::begin(range), std::end(range));
		sort_uniq();
		return *this;
	}

	auto replace(StrT& s) const -> StrT&;
	auto replace_copy(StrT s) const -> StrT;
};
using Substring_Replacer = Substr_Replacer<char>;
using WSubstring_Replacer = Substr_Replacer<wchar_t>;

template <class CharT>
class Break_Table {
      public:
	using StrT = std::basic_string<CharT>;
	using Table_Str = std::vector<StrT>;
	using iterator = typename Table_Str::iterator;
	using const_iterator = typename Table_Str::const_iterator;

      private:
	Table_Str table;
	iterator start_word_breaks_last_it;
	iterator end_word_breaks_last_it;

	auto order_entries() -> void;

      public:
	Break_Table() = default;
	Break_Table(const Table_Str& v);
	Break_Table(Table_Str&& v);

	auto operator=(const Table_Str& v) -> Break_Table&;
	auto operator=(Table_Str&& v) -> Break_Table&;
	template <class Range>
	auto operator=(const Range& range) -> Break_Table&
	{
		table.assign(std::begin(range), std::end(range));
		order_entries();
		return *this;
	}

	auto start_word_breaks() const -> boost::iterator_range<const_iterator>
	{
		return {std::begin(table),
		        const_iterator(start_word_breaks_last_it)};
	}
	auto end_word_breaks() const -> boost::iterator_range<const_iterator>
	{
		return {const_iterator(start_word_breaks_last_it),
		        const_iterator(end_word_breaks_last_it)};
	}
	auto middle_word_breaks() const -> boost::iterator_range<const_iterator>
	{
		return {const_iterator(end_word_breaks_last_it),
		        std::end(table)};
	}
};

template <class CharT>
class Char_Eraser {
      public:
	using StrT = std::basic_string<CharT>;

      private:
	StrT erase_chars;

	auto sort_uniq() -> void;
	auto lookup(CharT c) const -> bool;

      public:
	auto operator=(const StrT& e) -> Char_Eraser&;
	auto operator=(StrT&& e) -> Char_Eraser&;

	auto erase(StrT& s) const -> StrT&;
	auto erase_copy(const StrT& s) const -> StrT;
};

template <class CharT>
class Prefix_Entry {
      public:
	using StrT = std::basic_string<CharT>;
	using RegexT = std::basic_regex<CharT>;

	const char16_t flag;
	const bool cross_product;
	const StrT stripping;
	const StrT appending;
	const RegexT condition;

	Prefix_Entry() = default;
	Prefix_Entry(char16_t flag, bool cross_product, const StrT& strip,
	             const StrT& append, StrT condition);

	auto to_root(StrT& word) const -> StrT&;
	auto to_root_copy(StrT word) const -> StrT;

	auto to_derived(StrT& word) const -> StrT&;
	auto to_derived_copy(StrT word) const -> StrT;

	auto check_condition(const StrT& word) const -> bool;
};

template <class CharT>
class Suffix_Entry {
      public:
	using StrT = std::basic_string<CharT>;
	using RegexT = std::basic_regex<CharT>;

	const char16_t flag;
	const bool cross_product;
	const StrT stripping;
	const StrT appending;
	const RegexT condition;

	Suffix_Entry() = default;
	Suffix_Entry(char16_t flag, bool cross_product, const StrT& strip,
	             const StrT& append, StrT condition);

	auto to_root(StrT& word) const -> StrT&;
	auto to_root_copy(StrT word) const -> StrT;

	auto to_derived(StrT& word) const -> StrT&;
	auto to_derived_copy(StrT word) const -> StrT;

	auto check_condition(const StrT& word) const -> bool;
};

using boost::multi_index_container;
using boost::multi_index::indexed_by;
using boost::multi_index::member;
using boost::multi_index::hashed_non_unique;

template <class CharT>
using Prefix_Table = multi_index_container<
    Prefix_Entry<CharT>,
    indexed_by<hashed_non_unique<
        member<Prefix_Entry<CharT>, const std::basic_string<CharT>,
               &Prefix_Entry<CharT>::appending>>>>;

template <class CharT>
using Suffix_Table = multi_index_container<
    Suffix_Entry<CharT>,
    indexed_by<hashed_non_unique<
        member<Suffix_Entry<CharT>, const std::basic_string<CharT>,
               &Suffix_Entry<CharT>::appending>>>>;
}
#endif