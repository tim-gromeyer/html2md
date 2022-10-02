// Copyright (c) Tim Gromeyer
// Licensed under the MIT License - https://opensource.org/licenses/MIT

#pragma once

#include <functional>
#include <regex>  // NOLINT [build/c++11]
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <map>

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}


namespace html2md {

// Main class: HTML to Markdown converter
class Converter {
 public:
  ~Converter() = default;

  static std::string Convert(std::string *html) {
    auto *instance = new Converter(html);
    auto md = instance->Convert2Md();
    delete instance;

    return md;
  }

  void PrepareHtml() {
    ReplaceAll(&html_, "\t", " ");
    ReplaceAll(&html_, "&amp;", "&");
    ReplaceAll(&html_, "&nbsp;", " ");
    ReplaceAll(&html_, "&rarr;", "→");


    std::regex exp("<!--(.*?)-->");
    html_ = regex_replace(html_, exp, "");
  }

  void CleanUpMarkdown() {
    TidyAllLines(&md_);

    ReplaceAll(&md_, " , ", ", ");

    ReplaceAll(&md_, "\n.\n", ".\n");
    ReplaceAll(&md_, "\n↵\n", " ↵\n");
    ReplaceAll(&md_, "\n*\n", "\n");
    ReplaceAll(&md_, "\n. ", ".\n");

    ReplaceAll(&md_, " [ ", " [");
    ReplaceAll(&md_, "\n[ ", "\n[");

    ReplaceAll(&md_, "&quot;", "\"");
    ReplaceAll(&md_, "&lt;", "<");
  }

  Converter* AppendToMd(char ch) {
    if (IsInIgnoredTag()) return this;

    md_ += ch;

    if (ch == '\n')
      chars_in_curr_line_ = 0;
    else
      ++chars_in_curr_line_;

    return this;
  }

  Converter* AppendToMd(const char *str) {
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

  // Append ' ' if:
  // - md does not end w/ '**'
  // - md does not end w/ '\n'
  Converter* AppendBlank() {
    UpdatePrevChFromMd();

    if (prev_ch_in_md_ == '\n'
        || (prev_ch_in_md_ == '*' && prev_prev_ch_in_md_ == '*')) return this;

    return AppendToMd(' ');
  }

 private:
  // Attributes
  static constexpr const char *kAttributeHref = "href";
  static constexpr const char *kAttributeAlt = "alt";
  static constexpr const char *kAttributeTitle = "title";
  static constexpr const char *kAttributeClass = "class";
  static constexpr const char *kAttributeSrc = "src";
  static constexpr const char *kAttrinuteAlign = "align";

  static constexpr const char *kTagAnchor = "a";
  static constexpr const char *kTagBold = "b";
  static constexpr const char *kTagBreak = "br";
  static constexpr const char *kTagCode = "code";
  static constexpr const char *kTagDiv = "div";
  static constexpr const char *kTagHead = "head";
  static constexpr const char *kTagLink = "link";
  static constexpr const char *kTagListItem = "li";
  static constexpr const char *kTagMeta = "meta";
  static constexpr const char *kTagNav = "nav";
  static constexpr const char *kTagNoScript = "noscript";
  static constexpr const char *kTagOption = "option";
  static constexpr const char *kTagOrderedList = "ol";
  static constexpr const char *kTagParagraph = "p";
  static constexpr const char *kTagPre = "pre";
  static constexpr const char *kTagScript = "script";
  static constexpr const char *kTagSpan = "span";
  static constexpr const char *kTagStrong = "strong";
  static constexpr const char *kTagStrighthrought = "del";
  static constexpr const char *kTagStyle = "style";
  static constexpr const char *kTagTemplate = "template";
  static constexpr const char *kTagTitle = "title";
  static constexpr const char *kTagUnorderedList = "ul";
  static constexpr const char *kTagImg = "img";
  static constexpr const char *kTagItalic = "em";
  static constexpr const char *kTagUnderline = "u";
  static constexpr const char *kTagSeperator = "hr";

  // Header
  static constexpr const char *kTagHeader1 = "h1";
  static constexpr const char *kTagHeader2 = "h2";
  static constexpr const char *kTagHeader3 = "h3";
  static constexpr const char *kTagHeader4 = "h4";
  static constexpr const char *kTagHeader5 = "h5";
  static constexpr const char *kTagHeader6 = "h6";

  // Table
  static constexpr const char *kTagTable = "table";
  static constexpr const char *kTagTableRow = "tr";
  static constexpr const char *kTagTableHeader = "th";
  static constexpr const char *kTagTableData = "td";

  u_int16_t index_ch_in_html_ = 0;

  std::vector<std::string> dom_tags_;

  bool is_in_tag_ = false;
  bool is_closing_tag_ = false;
  bool is_in_attribute_value_ = false;
  bool is_in_pre_ = false;
  bool is_in_code_ = false;
  bool is_in_table_ = false;

  // relevant for <li> only, false = is in unordered list
  bool is_in_ordered_list_ = false;
  int index_li = 0;

  char prev_ch_in_md_, prev_prev_ch_in_md_ = 0;
  char prev_ch_in_html_ = 'x';

  std::string html_;

  u_int16_t offset_lt_;
  std::string current_tag_;
  std::string prev_tag_;

  std::string current_attribute_;
  std::string current_attribute_value_;

  std::string current_href_;
  std::string current_title_;

  std::string current_alt_;
  std::string current_src_;

  // Line which separates header from data
  std::string tableLine;

  size_t chars_in_curr_line_ = 0;
  u_int16_t char_index_in_tag_content = 0;

  std::string md_;
  size_t md_len_ = 0;

  // Tag: base class for tag types
  struct Tag {
    virtual void OnHasLeftOpeningTag(Converter* converter) = 0;
    virtual void OnHasLeftClosingTag(Converter* converter) = 0;
  };

  // Tag types

  // tags that are not printed (nav, script, noscript, ...)
  struct TagIgnored : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
    }
    void OnHasLeftClosingTag(Converter* converter) override {
    }
  };

  struct TagAnchor : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      if (converter->IsInIgnoredTag()) return;

      if (converter->prev_tag_ == kTagImg) converter->AppendToMd('\n');

      converter->current_title_ = converter->ExtractAttributeFromTagLeftOf(kAttributeTitle);

      converter->current_href_ =
          converter->RTrim(&converter->md_, true)
                   ->AppendBlank()
                   ->AppendToMd("[")
                   ->ExtractAttributeFromTagLeftOf(kAttributeHref);
          }

    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->IsInIgnoredTag()) return;

      if (converter->prev_ch_in_md_ == ' ') {
        converter->ShortenMarkdown();
      }

      if (converter->prev_ch_in_md_ == '[') {
        converter->ShortenMarkdown();
      } else {
        converter->AppendToMd("](")
                 ->AppendToMd(converter->current_href_.c_str());

        // If title is set append it
        if (!converter->current_title_.empty()) {
            converter->AppendToMd(" \"")
                     ->AppendToMd(converter->current_title_.c_str())
                     ->AppendToMd('"');
        }

        converter->AppendToMd(") ");

        if (converter->prev_tag_ == kTagImg)
            converter->AppendToMd('\n');
      }
    }
  };

  struct TagBold : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ != ' ') converter->AppendBlank();

      converter->AppendToMd("**");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

      converter->AppendToMd("** ");
    }
  };

  struct TagItalic : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ != ' ') converter->AppendBlank();

      converter->AppendToMd('*');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

      converter->AppendToMd("* ");
    }
  };

  struct TagUnderline : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      // if (converter->prev_ch_in_md_ != ' ') converter->AppendBlank();

      converter->AppendToMd("<u>");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      // if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

      converter->AppendToMd("</u> ");
    }
  };

  struct TagStrikethrought : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ != ' ') converter->AppendBlank();

      converter->AppendToMd('~');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

      converter->AppendToMd("~ ");
    }
  };

  struct TagBreak : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
        if (converter->is_in_table_) converter->AppendToMd("<br>");
        else if (converter->md_len_ > 0) converter->AppendToMd("  \n");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
    }
  };

  struct TagDiv : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ != '\n') converter->AppendToMd('\n');

      if (converter->prev_prev_ch_in_md_ != '\n') converter->AppendToMd('\n');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
    }
  };

  struct TagHeader1 : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
        converter->AppendToMd("\n# ");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_prev_ch_in_md_ != ' ') converter->AppendToMd('\n');
    }
  };

  struct TagHeader2 : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      converter->AppendToMd("\n## ");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_prev_ch_in_md_ != ' ') converter->AppendToMd('\n');
    }
  };

  struct TagHeader3 : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      converter->AppendToMd("\n### ");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_prev_ch_in_md_ != ' ') converter->AppendToMd('\n');
    }
  };

  struct TagHeader4 : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      converter->AppendToMd("\n#### ");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_prev_ch_in_md_ != ' ') converter->AppendToMd('\n');
    }
  };

  struct TagHeader5 : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      converter->AppendToMd("\n##### ");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_prev_ch_in_md_ != ' ') converter->AppendToMd('\n');
    }
  };

  struct TagHeader6 : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      converter->AppendToMd("\n###### ");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_prev_ch_in_md_ != ' ') converter->AppendToMd('\n');
    }
  };

  struct TagListItem : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ != '\n') converter->AppendToMd("\n");

      if (!converter->is_in_ordered_list_) {
        converter->AppendToMd("- ");
        return;
      }

      ++converter->index_li;

      std::string num = std::to_string(converter->index_li);
      num += ". ";
      converter->AppendToMd(num.c_str());
    }
    void OnHasLeftClosingTag(Converter* converter) override {
    }
  };

  struct TagOption : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->md_len_ > 0) converter->AppendToMd("  \n");
    }
  };

  struct TagOrderedList : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
        converter->is_in_ordered_list_ = true;
        converter->index_li = 0;

      converter->ReplacePreviousSpaceInLineByNewline();

      converter->AppendToMd('\n');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
        converter->is_in_ordered_list_ = false;

        converter->AppendToMd('\n');
    }
  };

  struct TagParagraph : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
        converter->AppendToMd('\n');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (!converter->md_.empty()) converter->AppendToMd('\n');
    }
  };

  struct TagPre : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      converter->is_in_pre_ = true;

      if (converter->prev_ch_in_md_ != '\n') converter->AppendToMd("\n");

      if (converter->prev_prev_ch_in_md_ != '\n') converter->AppendToMd("\n");

      converter->AppendToMd("```");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      converter->is_in_pre_ = false;

      converter->AppendToMd("```\n");
    }
  };

  struct TagCode : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      converter->is_in_code_ = true;

      if (converter->is_in_pre_) {
          // Remove space
          if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

          auto code = converter->ExtractAttributeFromTagLeftOf(kAttributeClass);
          if (!code.empty()) {
              code.erase(0, 9); // remove language-
              converter->AppendToMd(code.c_str());
          }
          converter->AppendToMd('\n');

          return;
      };

      converter->AppendToMd('`');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      converter->is_in_code_ = false;

      if (converter->is_in_pre_) return;

      if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

      converter->AppendToMd("` ");
    }
  };

  struct TagSpan : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ != ' '
          && converter->char_index_in_tag_content > 0)
        converter->AppendBlank();
    }
  };

  struct TagTitle : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      converter->TurnLineIntoHeader1();
    }
  };

  struct TagUnorderedList : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ != '\n') converter->AppendToMd("\n");

      if (converter->prev_prev_ch_in_md_ != '\n') converter->AppendToMd("\n");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
        converter->AppendToMd('\n');
    }
  };

  struct TagImage : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          if (converter->prev_tag_ != kTagAnchor) converter->ReplacePreviousSpaceInLineByNewline();

          converter->AppendToMd("![")
                   ->AppendToMd(converter->ExtractAttributeFromTagLeftOf(kAttributeAlt).c_str())
                   ->AppendToMd("](")
                   ->AppendToMd(converter->ExtractAttributeFromTagLeftOf(kAttributeSrc).c_str())
                   ->AppendToMd(")");
      }
      void OnHasLeftClosingTag(Converter* converter) override {
          if (converter->prev_tag_ == kTagAnchor) converter->AppendToMd('\n');
      }
  };

  struct TagSeperator : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          converter->AppendToMd('\n');
          converter->AppendToMd(Repeat("-", 3).c_str());
          converter->AppendToMd('\n');
      }
      void OnHasLeftClosingTag(Converter* converter) override {
      }
  };

  struct TagTable : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          converter->is_in_table_ = true;
          converter->AppendToMd('\n');
      }
      void OnHasLeftClosingTag(Converter* converter) override {
          converter->is_in_table_ = false;
          converter->AppendToMd('\n');
      }
  };

  struct TagTableRow : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          converter->AppendToMd('\n');
      }
      void OnHasLeftClosingTag(Converter* converter) override {
          if (endsWith(converter->md_, "|")) converter->AppendToMd('\n'); // There's a bug
          else converter->AppendToMd('|');

          if (!converter->tableLine.empty()) {
              if (!endsWith(converter->md_, "\n")) converter->AppendToMd('\n');

              converter->tableLine.append("|\n");
              converter->AppendToMd(converter->tableLine.c_str());
              converter->tableLine.clear();
          }
      }
  };

  struct TagTableHeader : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          auto align = converter->ExtractAttributeFromTagLeftOf(kAttrinuteAlign);

          std::string line = "| ";

          if (align == "left" || align == "center")
              line += ':';

          line += '-';

          if (align == "right" || align == "center")
              line += ": ";
          else
              line += ' ';

          converter->tableLine.append(line);

          converter->AppendToMd("| ");
      }
      void OnHasLeftClosingTag(Converter* converter) override {
          // converter->AppendToMd('\n');
      }
  };

  struct TagTableData : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          if (converter->prev_prev_ch_in_md_ != '|') converter->AppendToMd("| ");
      }
      void OnHasLeftClosingTag(Converter* converter) override {
          // converter->AppendToMd('\n');
      }
  };

  std::map<std::string, Tag*> tags_;

  explicit Converter(std::string *html) : html_(*html) {
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
    tags_[kTagItalic] = new TagItalic();
    tags_[kTagUnderline] = new TagUnderline();
    tags_[kTagStrighthrought] = new TagStrikethrought();

    // Tables
    tags_[kTagTable] = new TagTable();
    tags_[kTagTableRow] = new TagTableRow();
    tags_[kTagTableHeader] = new TagTableHeader();
    tags_[kTagTableData] = new TagTableData();
  }

  // Trim from start (in place)
  static void LTrim(std::string *s) {
    (*s).erase(
        (*s).begin(),
        std::find_if(
            (*s).begin(),
            (*s).end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
  }

  // Trim from end (in place)
  Converter * RTrim(std::string *s, bool trim_only_blank = false) {
    (*s).erase(
        std::find_if(
            (*s).rbegin(),
            (*s).rend(),
            trim_only_blank
              ? std::not1(std::ptr_fun<int, int>(std::isblank))
              : std::not1(std::ptr_fun<int, int>(std::isspace))
        )
            .base(),
        (*s).end());

    return this;
  }

  // Trim from both ends (in place)
  Converter * Trim(std::string *s) {
    LTrim(s);
    RTrim(s);

    return this;
  }

  // 1. trim all lines
  // 2. reduce consecutive newlines to maximum 3
  void TidyAllLines(std::string *str) {
    auto lines = Explode(*str, '\n');
    std::string res;

    int amount_newlines = 0;

    for (auto line : lines) {
      // Don't trim if it's a line break
      if (!endsWith(line, "  "))
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

  static int ReplaceAll(std::string *haystack,
                        const std::string &needle,
                        const std::string &replacement) {
    // Get first occurrence
    size_t pos = (*haystack).find(needle);

    int amount_replaced = 0;

    // Repeat until end is reached
    while (pos != std::string::npos) {
      // Replace this occurrence of sub string
      (*haystack).replace(pos, needle.size(), replacement);

      // Get the next occurrence from the current position
      pos = (*haystack).find(needle, pos + replacement.size());

      amount_replaced++;
    }

    return amount_replaced;
  }

  // Split given string by given character delimiter into vector of strings
  static std::vector<std::string> Explode(std::string const &str,
                                          char delimiter) {
    std::vector<std::string> result;
    std::istringstream iss(str);

    for (std::string token; std::getline(iss, token, delimiter);)
      result.push_back(std::move(token));

    return result;
  }

  // Repeat given amount of given string
  static std::string Repeat(const std::string &str, size_t amount) {
    std::string out;

    for (u_int16_t i = 0; i < amount; i++) {
      out += str;
    }

    return out;
  }

  std::string ExtractAttributeFromTagLeftOf(const std::string& attr) {
    char ch = ' ';

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

  void TurnLineIntoHeader1() {
    md_ += "\n" + Repeat("=", chars_in_curr_line_) + "\n\n";

    chars_in_curr_line_ = 0;
  }

  void TurnLineIntoHeader2() {
    md_ += "\n" + Repeat("-", chars_in_curr_line_) + "\n\n";

    chars_in_curr_line_ = 0;
  }

  // Main character iteration of parser
  std::string Convert2Md() {
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

  void UpdateMdLen() { md_len_ = md_.length(); }

  // Current char: '<'
  void OnHasEnteredTag() {
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

  Converter * UpdatePrevChFromMd() {
    if (!md_.empty()) {
      prev_ch_in_md_ = md_[md_.length() - 1];

      if (md_.length() > 1)
        prev_prev_ch_in_md_ = md_[md_.length() - 2];
    }

    return this;
  }

  /**
   * Handle next char within <...> tag
   *
   * @param ch current character
   * @return   continue surrounding iteration?
   */
  bool ParseCharInTag(char ch) {
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

  // Current char: '>'
  bool OnHasLeftTag() {
    is_in_tag_ = false;

    UpdateMdLen();
    UpdatePrevChFromMd();

    bool is_hidden = is_closing_tag_
                     ? false
                     : TagContainsAttributesToHide(&current_tag_);

    current_tag_ = Explode(current_tag_, ' ')[0];

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

  static bool TagContainsAttributesToHide(std::string *tag) {
    return (*tag).find(" aria=\"hidden\"") != std::string::npos
        || (*tag).find("display:none") != std::string::npos
        || (*tag).find("visibility:hidden") != std::string::npos
        || (*tag).find("opacity:0") != std::string::npos
        || (*tag).find("Details-content--hidden-not-important")
            != std::string::npos;
  }

  Converter* ShortenMarkdown(int chars = 1) {
    UpdateMdLen();

    md_ = md_.substr(0, md_len_ - chars);

    return this->UpdatePrevChFromMd();
  }

  /**
   * @param ch
   * @return continue iteration surrounding  this method's invocation?
   */
  bool ParseCharInTagContent(char ch) {
    if (is_in_code_) {
      md_ += ch;

      ++chars_in_curr_line_;
      ++char_index_in_tag_content;

      UpdatePrevChFromMd();

      return false;
    }

    if (ch == '\n') return true;

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

    if (chars_in_curr_line_ > 80) {
      if (ch == ' ') { // If the next char is - it will become a list
        md_ += "\n";
        chars_in_curr_line_ = 0;
      } else if (chars_in_curr_line_ > 100) {
          ReplacePreviousSpaceInLineByNewline();
      }
    }

    return false;
  }

  // Replace previous space (if any) in current markdown line by newline
  bool ReplacePreviousSpaceInLineByNewline() {
    if (current_tag_ == kTagParagraph && prev_tag_ != kTagCode && prev_tag_!= kTagPre) return false;

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

  static bool IsIgnoredTag(const std::string &tag) {
    return (tag[0] == '-'
        || kTagTemplate == tag
        || kTagStyle == tag
        || kTagScript == tag
        || kTagNoScript == tag
        || kTagNav == tag);

    // meta: not ignored to tolerate if closing is omitted
  }

  [[nodiscard]] bool IsInIgnoredTag() const {
    auto len = dom_tags_.size();

    for (int i = 0; i < len; ++i) {
      std::string tag = dom_tags_[i];

      if (kTagPre == tag
          || kTagTitle == tag) return false;

      if (IsIgnoredTag(tag)) return true;
    }

    return false;
  }
};  // Converter

// Static wrapper for simple invocation
std::string Convert(std::string html) {
  return html2md::Converter::Convert(&html);
}

}  // namespace html2md
