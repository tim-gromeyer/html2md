// Copyright (c) Tim Gromeyer
// Licensed under the MIT License - https://opensource.org/licenses/MIT

#include "html2md.h"

#include <functional>
#include <regex>  // NOLINT [build/c++11]
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <map>

using namespace std;
using namespace html2md::utils;

namespace html2md {

namespace utils {
static bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static int ReplaceAll(std::string *haystack,
                      const std::string &needle,
                      const std::string &replacement) {
  // Get first occurrence
  size_t pos = (*haystack).find(needle);

  int amount_replaced = 0;

  // Repeat until end is reached
  while (pos != std::string::npos) {
    // Replace this occurrence of sub std::string
    (*haystack).replace(pos, needle.size(), replacement);

    // Get the next occurrence from the current position
    pos = (*haystack).find(needle, pos + replacement.size());

    amount_replaced++;
  }

  return amount_replaced;
}

// Split given std::string by given character delimiter into vector of std::strings
static std::vector<std::string> Split(std::string const &str, char delimiter) {
  vector<std::string> result;
  std::stringstream iss(str);

  for (std::string token; getline(iss, token, delimiter);)
    result.push_back(move(token));

  return result;
}
}

Converter::Converter(std::string *html) : html_(*html) {
  PrepareHtml();

  // non-printing tags
  auto *tagIgnored = new TagIgnored();
  tags_[kTagHead] = tagIgnored;
  tags_[kTagMeta] = tagIgnored;
  tags_[kTagNav] = tagIgnored;
  tags_[kTagNoScript] = tagIgnored;
  tags_[kTagScript] = tagIgnored;
  tags_[kTagStyle] = tagIgnored;
  tags_[kTagTemplate] = tagIgnored;

  // printing tags
  tags_[kTagAnchor] = new TagAnchor();
  tags_[kTagBreak] = new TagBreak();
  tags_[kTagDiv] = new TagDiv();
  tags_[kTagHeader1] = new TagHeader1();
  tags_[kTagHeader2] = new TagHeader2();
  tags_[kTagHeader3] = new TagHeader3();
  tags_[kTagHeader4] = new TagHeader4();
  tags_[kTagHeader5] = new TagHeader5();
  tags_[kTagHeader6] = new TagHeader6();
  tags_[kTagListItem] = new TagListItem();
  tags_[kTagOption] = new TagOption();
  tags_[kTagOrderedList] = new TagOrderedList();
  tags_[kTagPre] = new TagPre();
  tags_[kTagCode] = new TagCode();
  tags_[kTagParagraph] = new TagParagraph();
  tags_[kTagSpan] = new TagSpan();
  tags_[kTagUnorderedList] = new TagUnorderedList();
  tags_[kTagTitle] = new TagTitle();
  tags_[kTagImg] = new TagImage();
  tags_[kTagSeperator] = new TagSeperator();

  // Text formatting
  auto *bold = new TagBold();
  tags_[kTagBold] = bold;
  tags_[kTagStrong] = bold;

  auto *italic = new TagItalic();
  tags_[kTagItalic] = italic;
  tags_[kTagItalic2] = italic;
  tags_[kTagDefinition] = italic;
  tags_[kTagCitation] = italic;

  tags_[kTagUnderline] = new TagUnderline();

  auto *strikethrough = new TagStrikethrought();
  tags_[kTagStrighthrought] = strikethrough;
  tags_[kTagStrighthrought2] = strikethrough;

  tags_[kTagBlockquote] = new TagBlockquote();

  // Tables
  tags_[kTagTable] = new TagTable();
  tags_[kTagTableRow] = new TagTableRow();
  tags_[kTagTableHeader] = new TagTableHeader();
  tags_[kTagTableData] = new TagTableData();
}

void Converter::PrepareHtml() {
    ReplaceAll(&html_, "\t", " ");
    ReplaceAll(&html_, "&amp;", "&");
    ReplaceAll(&html_, "&nbsp;", " ");
    ReplaceAll(&html_, "&rarr;", "→");


    regex exp("<!--(.*?)-->");
    html_ = regex_replace(html_, exp, "");
}

void Converter::CleanUpMarkdown() {
    TidyAllLines(&md_);

    ReplaceAll(&md_, " , ", ", ");

    ReplaceAll(&md_, "\n.\n", ".\n");
    ReplaceAll(&md_, "\n↵\n", " ↵\n");
    ReplaceAll(&md_, "\n*\n", "\n");
    ReplaceAll(&md_, "\n. ", ".\n");

    ReplaceAll(&md_, " [ ", " [");
    ReplaceAll(&md_, "\n[ ", "\n[");

    ReplaceAll(&md_, "&quot;", R"(")");
    ReplaceAll(&md_, "&lt;", "<");
    ReplaceAll(&md_, "&gt;", ">");

    ReplaceAll(&md_, "\t\t  ", "\t\t");
    ReplaceAll(&md_, " </u> ", "</u>");
}

Converter* Converter::AppendToMd(char ch) {
    if (IsInIgnoredTag()) return this;

    md_ += ch;

    if (ch == '\n')
      chars_in_curr_line_ = 0;
    else
      ++chars_in_curr_line_;

    return this;
}

Converter* Converter::AppendToMd(const char *str) {
  if (IsInIgnoredTag()) return this;

  md_ += str;

  auto str_len = strlen(str);

  for (int i = 0; i < str_len; ++i) {
    if (str[i] == '\n')
      chars_in_curr_line_ = 0;
    else
      ++chars_in_curr_line_;
  }

  return this;
}

Converter* Converter::AppendBlank() {
  UpdatePrevChFromMd();

  if (prev_ch_in_md_ == '\n'
      || (prev_ch_in_md_ == '*' && prev_prev_ch_in_md_ == '*')) return this;

  return AppendToMd(' ');
}

void Converter::LTrim(std::string *s) {
  (*s).erase(
      (*s).begin(),
      find_if(
          (*s).begin(),
          (*s).end(),
          not1(ptr_fun<int, int>(isspace))));
}

Converter* Converter::RTrim(std::string *s, bool trim_only_blank) {
  (*s).erase(
      find_if(
          (*s).rbegin(),
          (*s).rend(),
          trim_only_blank
            ? not1(ptr_fun<int, int>(isblank))
            : not1(ptr_fun<int, int>(isspace))
      )
          .base(),
      (*s).end());

  return this;
}

Converter* Converter::Trim(std::string *s) {
  if (!startsWith(*s, "\t"))
    LTrim(s);

  if (!endsWith(*s, "  "))
    RTrim(s);

  return this;
}

void Converter::TidyAllLines(std::string *str) {
  auto lines = Split(*str, '\n');
  std::string res;

  int amount_newlines = 0;

  for (auto line : lines) {
    Trim(&line);

    if (line.empty()) {
      if (amount_newlines < 2 && !res.empty()) {
        res += "\n";
        amount_newlines++;
      }
    } else {
      amount_newlines = 0;

      res += line + "\n";
    }
  }

  *str = res;
}

std::string Converter::ExtractAttributeFromTagLeftOf(const std::string& attr) {
  // Extract the whole tag from current offset, e.g. from '>', backwards
  auto tag = html_.substr(offset_lt_, index_ch_in_html_ - offset_lt_);

  // locate given attribute
  auto offset_attr = tag.find(attr);

  if (offset_attr == std::string::npos) return "";

  // locate attribute-value pair's '='
  auto offset_equals = tag.find('=', offset_attr);

  if (offset_equals == std::string::npos) return "";

  // locate value's surrounding quotes
  auto offset_double_quote = tag.find('"', offset_equals);
  auto offset_single_quote = tag.find('\'', offset_equals);

  bool has_double_quote = offset_double_quote != std::string::npos;
  bool has_single_quote = offset_single_quote != std::string::npos;

  if (!has_double_quote && !has_single_quote) return "";

  char wrapping_quote = 0;

  u_int64_t offset_opening_quote = 0;
  u_int64_t offset_closing_quote = 0;

  if (has_double_quote) {
    if (!has_single_quote) {
      wrapping_quote = '"';
      offset_opening_quote = offset_double_quote;
    } else {
      if (offset_double_quote < offset_single_quote) {
        wrapping_quote = '"';
        offset_opening_quote = offset_double_quote;
      } else {
        wrapping_quote = '\'';
        offset_opening_quote = offset_single_quote;
      }
    }
  } else {
    // has only single quote
    wrapping_quote = '\'';
    offset_opening_quote = offset_single_quote;
  }

  if (offset_opening_quote == std::string::npos) return "";

  offset_closing_quote = tag.find(wrapping_quote, offset_opening_quote + 1);

  if (offset_closing_quote == std::string::npos) return "";

  return tag.substr(
      offset_opening_quote + 1,
      offset_closing_quote - 1 - offset_opening_quote);
}

void Converter::TurnLineIntoHeader1() {
  md_ += "\n" + Repeat("=", chars_in_curr_line_) + "\n\n";

  chars_in_curr_line_ = 0;
}

void Converter::TurnLineIntoHeader2() {
  md_ += "\n" + Repeat("-", chars_in_curr_line_) + "\n\n";

  chars_in_curr_line_ = 0;
}

std::string Converter::Convert2Md() {
  for (char ch : html_) {
    ++index_ch_in_html_;

    if (!is_in_tag_
        && ch == '<') {
      OnHasEnteredTag();

      continue;
    }

    UpdateMdLen();

    if ((is_in_tag_ && ParseCharInTag(ch))
        || (!is_in_tag_ && ParseCharInTagContent(ch)))
      continue;
  }

  CleanUpMarkdown();

  return md_;
}

void Converter::OnHasEnteredTag() {
  offset_lt_ = index_ch_in_html_;
  is_in_tag_ = true;
  prev_tag_ = current_tag_;
  current_tag_ = "";

  if (!md_.empty()) {
    UpdatePrevChFromMd();

    if (prev_ch_in_md_ != ' ' && prev_ch_in_md_ != '\n')
      md_ += ' ';
  }
}

Converter* Converter::UpdatePrevChFromMd() {
  if (!md_.empty()) {
    prev_ch_in_md_ = md_[md_.length() - 1];

    if (md_.length() > 1)
      prev_prev_ch_in_md_ = md_[md_.length() - 2];
  }

  return this;
}

bool Converter::ParseCharInTag(char ch) {
  if (ch == '/' && current_tag_.empty()) {
    is_closing_tag_ = true;

    return true;
  }

  if (ch == '>') return OnHasLeftTag();

  if (ch == '=') return true;

  if (ch == '"') {
    if (is_in_attribute_value_) {
      is_in_attribute_value_ = false;
      current_attribute_ = "";
      current_attribute_value_ = "";
    } else if (prev_ch_in_md_ == '=') {
      is_in_attribute_value_ = true;
    }

    return true;
  }

  if (is_in_attribute_value_) {
    current_attribute_value_ += ch;
  } else {
    current_attribute_ += ch;
  }

  current_tag_ += ch;

  return false;
}

bool Converter::OnHasLeftTag() {
  is_in_tag_ = false;

  UpdateMdLen();
  UpdatePrevChFromMd();

  bool is_hidden = is_closing_tag_
                   ? false
                   : TagContainsAttributesToHide(&current_tag_);

  current_tag_ = Split(current_tag_, ' ')[0];

  char_index_in_tag_content = 0;

  Tag* tag = tags_[current_tag_];

  if (tag) {
    if (is_closing_tag_) {
      is_closing_tag_ = false;

      tag->OnHasLeftClosingTag(this);

      if (!dom_tags_.empty()) {
        if (dom_tags_[dom_tags_.size() - 1] == kTagPre)
          is_in_pre_ = false;

        dom_tags_.pop_back();
      }
    } else {
      auto breadcrump_tag = current_tag_;

      if (is_hidden) breadcrump_tag = "-" + current_tag_;

      dom_tags_.push_back(breadcrump_tag);

      tag->OnHasLeftOpeningTag(this);
    }
  }

  return true;
}

Converter* Converter::ShortenMarkdown(int chars) {
  UpdateMdLen();

  md_ = md_.substr(0, md_len_ - chars);

  return this->UpdatePrevChFromMd();
}

bool Converter::ParseCharInTagContent(char ch) {
  if (is_in_code_) {
    md_ += ch;

    ++chars_in_curr_line_;
    ++char_index_in_tag_content;

    UpdatePrevChFromMd();

    return false;
  }

  if (ch == '\n') {
    if (prev_tag_ == kTagBlockquote && current_tag_ == kTagParagraph) {
      md_ += "\n";
      chars_in_curr_line_ = 0;
      AppendToMd(Repeat("> ", index_blockquote));
    }

    return true;
  }

  // if (ch == '\n') return true;

  if (IsInIgnoredTag()
      || current_tag_ == kTagLink) {
    prev_ch_in_html_ = ch;

    return true;
  }

  UpdatePrevChFromMd();

  // prevent more than one consecutive spaces
  if (ch == ' ') {
    if (md_len_ == 0
        || char_index_in_tag_content == 0
        || prev_ch_in_md_ == ' '
        || prev_ch_in_md_ == '\n')
      return true;
  }

  if (ch == '.' && prev_ch_in_md_ == ' ') {
    ShortenMarkdown();
    --chars_in_curr_line_;
  }

  md_ += ch;

  ++chars_in_curr_line_;
  ++char_index_in_tag_content;

  if (chars_in_curr_line_ > 80  && !is_in_table_ && !is_in_list_) {
    if (ch == ' ') { // If the next char is - it will become a list
      md_ += "\n";
      chars_in_curr_line_ = 0;
    } else if (chars_in_curr_line_ > 100) {
        ReplacePreviousSpaceInLineByNewline();
    }
  }

  return false;
}

bool Converter::ReplacePreviousSpaceInLineByNewline() {
  if (current_tag_ == kTagParagraph && (prev_tag_ != kTagCode && prev_tag_!= kTagPre) || is_in_table_) return false;

  UpdateMdLen();
  auto offset = md_len_ - 1;

  do {
    if (md_[offset] == '\n') return false;

    if (md_[offset] == ' ') {
      md_[offset] = '\n';
      chars_in_curr_line_ = md_len_ - offset;

      return true;
    }

    --offset;
  } while (offset > 0);

  return false;
}

bool Converter::ok() {
    return !(is_in_code_ || is_in_list_ || is_in_table_ || index_blockquote > 0);
}

}
