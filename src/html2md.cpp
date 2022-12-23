// Copyright (c) Tim Gromeyer
// Licensed under the MIT License - https://opensource.org/licenses/MIT

#include "html2md.h"

#include <cstring>
#include <functional>
#include <regex>  // NOLINT [build/c++11]
#include <sstream>
#include <stack>
#include <utility>

using std::string;
using std::vector;
using std::regex;

static bool startsWith(const string& str, const string& prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static bool endsWith(const string& str, const string& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static int ReplaceAll(string *haystack,
                      const string &needle,
                      const string &replacement) {
  // Get first occurrence
  size_t pos = (*haystack).find(needle);

  int amount_replaced = 0;

  // Repeat until end is reached
  while (pos != string::npos) {
    // Replace this occurrence of sub string
    (*haystack).replace(pos, needle.size(), replacement);

    // Get the next occurrence from the current position
    pos = (*haystack).find(needle, pos + replacement.size());

    ++amount_replaced;
  }

  return amount_replaced;
}

// Split given string by given character delimiter into vector of strings
static std::vector<string> Split(string const &str, char delimiter) {
  vector<string> result;
  std::stringstream iss(str);

  for (string token; getline(iss, token, delimiter);)
    result.push_back(token);

  return result;
}

static string Repeat(const string &str, size_t amount) {
  if (amount == 0) return "";
  else if (amount == 1) return str;

  string out;

  for (int_fast16_t i = 0; i < amount; ++i) out.append(str);

  return out;
}

namespace html2md {

Converter::Converter(string *html, options *options)
    : html_(*html), option(options) {
  options ? option = options
          : option = new struct options();

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
  tags_[kTagBold] = new TagBold();
  tags_[kTagStrong] = tags_[kTagBold];

  tags_[kTagItalic] = new TagItalic();
  tags_[kTagItalic2] = tags_[kTagItalic];
  tags_[kTagDefinition] = tags_[kTagItalic];
  tags_[kTagCitation] = tags_[kTagItalic];

  tags_[kTagUnderline] = new TagUnderline();

  tags_[kTagStrighthrought] = new TagStrikethrought();
  tags_[kTagStrighthrought2] = tags_[kTagStrighthrought];

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
    ReplaceAll(&md_, "[ ![", "[![");

    ReplaceAll(&md_, "&quot;", R"(")");
    ReplaceAll(&md_, "&lt;", "<");
    ReplaceAll(&md_, "&gt;", ">");

    ReplaceAll(&md_, "\t\t  ", "\t\t");
    ReplaceAll(&md_, " </u> ", "</u>");
}

Converter* Converter::appendToMd(char ch) {
    if (IsInIgnoredTag()) return this;

    md_ += ch;
    ++md_len_;

    if (ch == '\n')
      chars_in_curr_line_ = 0;
    else
      ++chars_in_curr_line_;

    return this;
}

Converter* Converter::appendToMd(const char *str) {
  if (IsInIgnoredTag()) return this;

  md_ += str;

  auto str_len = strlen(str);

  md_len_ += str_len;

  for (int i = 0; i < str_len; ++i) {
    if (str[i] == '\n')
      chars_in_curr_line_ = 0;
    else
      ++chars_in_curr_line_;
  }

  return this;
}

Converter* Converter::appendBlank() {
  UpdatePrevChFromMd();

  if (prev_ch_in_md_ == '\n'
      || (prev_ch_in_md_ == '*' && prev_prev_ch_in_md_ == '*')) return this;

  return appendToMd(' ');
}

bool Converter::ok() const
{
    return
      !is_in_pre_ &&
      !is_in_list_ &&
      !is_in_p_ &&
      !is_in_table_ &&
      !is_in_tag_ &&
      index_blockquote == 0 &&
      index_li == 0;
}

void Converter::LTrim(string *s) {
  (*s).erase((*s).begin(), find_if((*s).begin(), (*s).end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
}

Converter* Converter::RTrim(string *s, bool trim_only_blank) {
  (*s).erase(find_if((*s).rbegin(), (*s).rend(), [trim_only_blank](unsigned char ch) {
    if (trim_only_blank)
      return !isblank(ch);

    return !isspace(ch);
  }).base(), (*s).end());

  return this;
}

Converter* Converter::Trim(string *s) {
  if (!startsWith(*s, "\t"))
    LTrim(s);

  if (!(startsWith(*s, "  "), endsWith(*s, "  ")))
    RTrim(s);

  return this;
}

void Converter::TidyAllLines(string *str) {
  auto lines = Split(*str, '\n');
  string res;

  int amount_newlines = 0;

  for (auto line : lines) {
    Trim(&line);

    if (line.empty()) {
      if (amount_newlines < 2 && !res.empty()) {
        res += '\n';
        amount_newlines++;
      }
    } else {
      amount_newlines = 0;

      res += line + '\n';
    }
  }

  *str = res;
}

string Converter::ExtractAttributeFromTagLeftOf(const string& attr) {
  // Extract the whole tag from current offset, e.g. from '>', backwards
  auto tag = html_.substr(offset_lt_, index_ch_in_html_ - offset_lt_);

  // locate given attribute
  auto offset_attr = tag.find(attr);

  if (offset_attr == string::npos) return "";

  // locate attribute-value pair's '='
  auto offset_equals = tag.find('=', offset_attr);

  if (offset_equals == string::npos) return "";

  // locate value's surrounding quotes
  auto offset_double_quote = tag.find('"', offset_equals);
  auto offset_single_quote = tag.find('\'', offset_equals);

  bool has_double_quote = offset_double_quote != string::npos;
  bool has_single_quote = offset_single_quote != string::npos;

  if (!has_double_quote && !has_single_quote) return "";

  char wrapping_quote = 0;

  uint64_t offset_opening_quote = 0;
  uint64_t offset_closing_quote = 0;

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

  if (offset_opening_quote == string::npos) return "";

  offset_closing_quote = tag.find(wrapping_quote, offset_opening_quote + 1);

  if (offset_closing_quote == string::npos) return "";

  return tag.substr(
      offset_opening_quote + 1,
      offset_closing_quote - 1 - offset_opening_quote);
}

void Converter::TurnLineIntoHeader1() {
  appendToMd('\n' + Repeat("=", chars_in_curr_line_) + "\n\n");

  chars_in_curr_line_ = 0;
}

void Converter::TurnLineIntoHeader2() {
  appendToMd('\n' + Repeat("-", chars_in_curr_line_) + "\n\n");

  chars_in_curr_line_ = 0;
}

string Converter::Convert2Md() {
  if (index_ch_in_html_ == html_.size()) return md_;

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

Converter* Converter::ShortenMarkdown(size_t chars) {
  UpdateMdLen();

  md_ = md_.substr(0, md_len_ - chars);

  if (chars > chars_in_curr_line_) chars_in_curr_line_ = 0;
  else chars_in_curr_line_ = chars_in_curr_line_ - chars;

  return this->UpdatePrevChFromMd();
}

bool Converter::ParseCharInTagContent(char ch) {
  if (is_in_code_) {
    md_ += ch;
    ++md_len_;


    ++chars_in_curr_line_;
    ++char_index_in_tag_content;

    UpdatePrevChFromMd();

    return false;
  }

  if (ch == '\n') {
    if (prev_tag_ == kTagBlockquote && current_tag_ == kTagParagraph) {
      md_ += '\n';
      ++md_len_;
      chars_in_curr_line_ = 0;
      appendToMd(Repeat("> ", index_blockquote));
    }

    return true;
  }

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
  ++md_len_;

  ++chars_in_curr_line_;
  ++char_index_in_tag_content;

  if (chars_in_curr_line_ > 80  && !is_in_table_ && !is_in_list_ &&
          current_tag_ != kTagImg && current_tag_ != kTagAnchor &&
          option->splitLines) {
    if (ch == ' ') { // If the next char is - it will become a list
      md_ += '\n';
      ++md_len_;
      chars_in_curr_line_ = 0;
    } else if (chars_in_curr_line_ > 100) {
        ReplacePreviousSpaceInLineByNewline();
    }
  }

  return false;
}

bool Converter::ReplacePreviousSpaceInLineByNewline() {
  if (current_tag_ == kTagParagraph || is_in_table_ &&
      (prev_tag_ != kTagCode && prev_tag_!= kTagPre)) return false;

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

void Converter::TagAnchor::OnHasLeftOpeningTag(Converter *converter) {
    if (converter->IsInIgnoredTag()) return;

    if (converter->prev_tag_ == kTagImg) converter->appendToMd('\n');

    converter->current_title_ = converter->ExtractAttributeFromTagLeftOf(kAttributeTitle);

    converter->current_href_ =
            converter->RTrim(&converter->md_, true)
            ->appendBlank()
            ->appendToMd("[")
            ->ExtractAttributeFromTagLeftOf(kAttributeHref);
}

void Converter::TagAnchor::OnHasLeftClosingTag(Converter *converter) {
    if (converter->IsInIgnoredTag()) return;

    if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

    if (converter->prev_ch_in_md_ == '[') converter->ShortenMarkdown();
    else {
        converter->appendToMd("](")
                 ->appendToMd(converter->current_href_);

        // If title is set append it
        if (!converter->current_title_.empty()) {
            converter->appendToMd(" \"")
                     ->appendToMd(converter->current_title_)
                     ->appendToMd('"');
        }

        converter->appendToMd(") ");

        if (converter->prev_tag_ == kTagImg)
            converter->appendToMd('\n');
    }
}

void Converter::TagBold::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd("**");
}

void Converter::TagBold::OnHasLeftClosingTag(Converter *converter) {
    converter->shortIfPrevCh(' ');

    converter->appendToMd("** ");
}

void Converter::TagItalic::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd('*');
}

void Converter::TagItalic::OnHasLeftClosingTag(Converter *converter) {
    converter->shortIfPrevCh(' ');

    converter->appendToMd("* ");
}

void Converter::TagUnderline::OnHasLeftOpeningTag(Converter *converter) {
    if (converter->prev_prev_ch_in_md_ == ' ' &&
            converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

    converter->appendToMd("<u>");
}

void Converter::TagUnderline::OnHasLeftClosingTag(Converter *converter) {
    converter->shortIfPrevCh(' ');

    converter->appendToMd("</u>");
}

void Converter::TagStrikethrought::OnHasLeftOpeningTag(Converter *converter) {
    converter->shortIfPrevCh(' ');

    converter->appendToMd('~');
}

void Converter::TagStrikethrought::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

    converter->appendToMd("~ ");
}

void Converter::TagBreak::OnHasLeftOpeningTag(Converter *converter) {
    if(converter->is_in_list_) { // When it's in a list, it's not in a paragraph
        converter->appendToMd("  \n");
        converter->appendToMd(Repeat("  ", converter->index_li));
    } else if (converter->is_in_table_) {
        if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

        converter->appendToMd("<br>");
    } else if (!converter->is_in_p_) {
        converter->appendToMd("\n<br>\n\n");
    } else if (converter->md_len_ > 0) converter->appendToMd("  \n");

    converter->appendToMd(Repeat("> ", converter->index_blockquote));
}

void Converter::TagBreak::OnHasLeftClosingTag(Converter *converter) {}

void Converter::TagDiv::OnHasLeftOpeningTag(Converter *converter) {
    if (converter->prev_ch_in_md_ != '\n') converter->appendToMd('\n');

    if (converter->prev_prev_ch_in_md_ != '\n') converter->appendToMd('\n');
}

void Converter::TagDiv::OnHasLeftClosingTag(Converter *converter) {}

void Converter::TagHeader1::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd("\n# ");
}

void Converter::TagHeader1::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_prev_ch_in_md_ != ' ') converter->appendToMd('\n');
}

void Converter::TagHeader2::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd("\n## ");
}

void Converter::TagHeader2::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_prev_ch_in_md_ != ' ') converter->appendToMd('\n');
}

void Converter::TagHeader3::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd("\n### ");
}

void Converter::TagHeader3::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_prev_ch_in_md_ != ' ') converter->appendToMd('\n');
}

void Converter::TagHeader4::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd("\n#### ");
}

void Converter::TagHeader4::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_prev_ch_in_md_ != ' ') converter->appendToMd('\n');
}

void Converter::TagHeader5::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd("\n##### ");
}

void Converter::TagHeader5::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_prev_ch_in_md_ != ' ') converter->appendToMd('\n');
}

void Converter::TagHeader6::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd("\n###### ");
}

void Converter::TagHeader6::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_prev_ch_in_md_ != ' ') converter->appendToMd('\n');
}

void Converter::TagListItem::OnHasLeftOpeningTag(Converter *converter) {
    if (converter->is_in_table_) return;

    if (!converter->is_in_ordered_list_) {
        converter->appendToMd(string({converter->option->unorderedList, ' '}));
        return;
    }

    ++converter->index_ol;

    string num = std::to_string(converter->index_ol);
    num.append({converter->option->orderedList, ' '});
    converter->appendToMd(num);
}

void Converter::TagListItem::OnHasLeftClosingTag(Converter *converter) {
    if (converter->is_in_table_) return;

    if (converter->prev_ch_in_md_ != '\n') converter->appendToMd('\n');
}

void Converter::TagOption::OnHasLeftOpeningTag(Converter *converter) {}

void Converter::TagOption::OnHasLeftClosingTag(Converter *converter) {
    if (converter->md_len_ > 0) converter->appendToMd("  \n");
}

void Converter::TagOrderedList::OnHasLeftOpeningTag(Converter *converter) {
    if (converter->is_in_table_) return;

    converter->is_in_list_ = true;
    converter->is_in_ordered_list_ = true;
    converter->index_ol = 0;

    ++converter->index_li;

    converter->ReplacePreviousSpaceInLineByNewline();

    converter->appendToMd('\n');
}

void Converter::TagOrderedList::OnHasLeftClosingTag(Converter *converter) {
    if (converter->is_in_table_) return;

    converter->is_in_ordered_list_ = false;

    if (converter->index_li != 0)
        --converter->index_li;

    converter->is_in_list_ = converter->index_li != 0;

    converter->appendToMd('\n');
}

void Converter::TagParagraph::OnHasLeftOpeningTag(Converter *converter) {
    converter->is_in_p_ = true;

    if (converter->is_in_list_ && converter->prev_tag_ == kTagParagraph)
        converter->appendToMd("\n\t");
    else if (!converter->is_in_list_ && converter->index_blockquote == 0)
        converter->appendToMd('\n');

    if (converter->index_blockquote > 0) {
        converter->appendToMd("> \n");
        converter->appendToMd(Repeat("> ", converter->index_blockquote));
    }
}

void Converter::TagParagraph::OnHasLeftClosingTag(Converter *converter) {
    converter->is_in_p_ = false;

    if (!converter->md_.empty()) converter->appendToMd('\n');
}

void Converter::TagPre::OnHasLeftOpeningTag(Converter *converter) {
    converter->is_in_pre_ = true;

    if (converter->prev_ch_in_md_ != '\n') converter->appendToMd("\n");

    if (converter->prev_prev_ch_in_md_ != '\n') converter->appendToMd("\n");

    if (converter->index_blockquote != 0) {
        converter->appendToMd(Repeat("> ", converter->index_blockquote));
        converter->ShortenMarkdown();
    }

    if (converter->is_in_list_ && converter->prev_tag_ != kTagParagraph) converter->ShortenMarkdown(2);

    if (converter->is_in_list_ || converter->index_blockquote != 0)
        converter->appendToMd("\t\t");
    else
        converter->appendToMd("```");
}

void Converter::TagPre::OnHasLeftClosingTag(Converter *converter) {
    converter->is_in_pre_ = false;

    if (!converter->is_in_list_ && converter->index_blockquote == 0)
        converter->appendToMd("```\n");
}

void Converter::TagCode::OnHasLeftOpeningTag(Converter *converter) {
    converter->is_in_code_ = true;

    if (converter->is_in_pre_) {
        // Remove space
        if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

        if (converter->is_in_list_ || converter->index_blockquote != 0) return;

        auto code = converter->ExtractAttributeFromTagLeftOf(kAttributeClass);
        if (!code.empty()) {
            if (startsWith(code, "language-"))
                code.erase(0, 9); // remove language-
            converter->appendToMd(code);
        }
        converter->appendToMd('\n');
    }
    else converter->appendToMd('`');
}

void Converter::TagCode::OnHasLeftClosingTag(Converter *converter) {
    converter->is_in_code_ = false;

    if (converter->is_in_pre_) return;

    if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

    converter->appendToMd("` ");
}

void Converter::TagSpan::OnHasLeftOpeningTag(Converter *converter) {}

void Converter::TagSpan::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_ch_in_md_ != ' '
            && converter->char_index_in_tag_content > 0)
        converter->appendBlank();
}

void Converter::TagTitle::OnHasLeftOpeningTag(Converter *converter) {}

void Converter::TagTitle::OnHasLeftClosingTag(Converter *converter) {
    converter->TurnLineIntoHeader1();
}

void Converter::TagUnorderedList::OnHasLeftOpeningTag(Converter *converter) {
    if (converter->is_in_list_ || converter->is_in_table_) return;

    converter->is_in_list_ = true;

    ++converter->index_li;

    converter->appendToMd('\n');
}

void Converter::TagUnorderedList::OnHasLeftClosingTag(Converter *converter) {
    if (converter->is_in_table_) return;

    if(converter->index_li != 0)
        --converter->index_li;

    converter->is_in_list_ = converter->index_li != 0;

    if (converter->prev_prev_ch_in_md_ == '\n' && converter->prev_ch_in_md_ == '\n')
        converter->ShortenMarkdown();
    else if (converter->prev_ch_in_md_ != '\n') converter->appendToMd('\n');
}

void Converter::TagImage::OnHasLeftOpeningTag(Converter *converter) {
    if (converter->prev_tag_ != kTagAnchor && converter->prev_ch_in_md_ != '\n')
        converter->appendToMd('\n');

    converter->appendToMd("![")
             ->appendToMd(converter->ExtractAttributeFromTagLeftOf(kAttributeAlt))
             ->appendToMd("](")
             ->appendToMd(converter->ExtractAttributeFromTagLeftOf(kAttributeSrc))
             ->appendToMd(")");
}

void Converter::TagImage::OnHasLeftClosingTag(Converter *converter) {
    if (converter->prev_tag_ == kTagAnchor) converter->appendToMd('\n');
}

void Converter::TagSeperator::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd("\n---\n");
}

void Converter::TagSeperator::OnHasLeftClosingTag(Converter *converter) {}

void Converter::TagTable::OnHasLeftOpeningTag(Converter *converter) {
    converter->is_in_table_ = true;
    converter->appendToMd('\n');
    converter->table_start = converter->md_len_;
}

void Converter::TagTable::OnHasLeftClosingTag(Converter *converter) {
    converter->is_in_table_ = false;
    converter->appendToMd('\n');
}

void Converter::TagTableRow::OnHasLeftOpeningTag(Converter *converter) {
    converter->appendToMd('\n');
}

void Converter::TagTableRow::OnHasLeftClosingTag(Converter *converter) {
    converter->UpdatePrevChFromMd();
    if (converter->prev_ch_in_md_ == '|') converter->appendToMd('\n'); // There's a bug
    else converter->appendToMd('|');

    if (!converter->tableLine.empty()) {
        if (converter->prev_ch_in_md_ != '\n') converter->appendToMd('\n');

        converter->tableLine.append("|\n");
        converter->appendToMd(converter->tableLine);
        converter->tableLine.clear();
    }
}

void Converter::TagTableHeader::OnHasLeftOpeningTag(Converter *converter) {
    auto align = converter->ExtractAttributeFromTagLeftOf(kAttrinuteAlign);

    string line = "| ";

    if (align == "left" || align == "center") line += ':';

    line += '-';

    if (align == "right" || align == "center") line += ": ";
    else line += ' ';

    converter->tableLine.append(line);

    converter->appendToMd("| ");
}

void Converter::TagTableHeader::OnHasLeftClosingTag(Converter *converter) {}

void Converter::TagTableData::OnHasLeftOpeningTag(Converter *converter) {
    if (converter->prev_prev_ch_in_md_ != '|') converter->appendToMd("| ");
}

void Converter::TagTableData::OnHasLeftClosingTag(Converter *converter) {}

void Converter::TagBlockquote::OnHasLeftOpeningTag(Converter *converter) {
    ++converter->index_blockquote;

    if (converter->index_blockquote == 1) converter->appendToMd('\n');
}

void Converter::TagBlockquote::OnHasLeftClosingTag(Converter *converter) {
    --converter->index_blockquote;
}

Converter::~Converter() = default;

} // namespace html2md
