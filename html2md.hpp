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

using namespace std;

static bool endsWith(const string& str, const string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const string& str, const string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}


namespace html2md {

// Main class: HTML to Markdown converter
class Converter {
 public:
  explicit Converter(string &html) {
        *this = Converter(&html);
  }

  ~Converter() = default;

  string Convert() { return Convert2Md(); }

  static string Convert(string *html) {
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


    regex exp("<!--(.*?)-->");
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

    ReplaceAll(&md_, "&quot;", R"(\")");
    ReplaceAll(&md_, "&lt;", "<");
    ReplaceAll(&md_, "&gt;", ">");

    ReplaceAll(&md_, "\t\t  ", "\t\t");
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

  Converter* AppendToMd(const string &s) {
    return AppendToMd(s.c_str());
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
  static constexpr const char *kTagStyle = "style";
  static constexpr const char *kTagTemplate = "template";
  static constexpr const char *kTagTitle = "title";
  static constexpr const char *kTagUnorderedList = "ul";
  static constexpr const char *kTagImg = "img";
  static constexpr const char *kTagSeperator = "hr";

  // Text format
  static constexpr const char *kTagBold = "b";
  static constexpr const char *kTagStrong = "strong";
  static constexpr const char *kTagItalic = "em";
  static constexpr const char *kTagItalic2 = "i";
  static constexpr const char *kTagCitation = "cite";
  static constexpr const char *kTagDefinition = "dfn";
  static constexpr const char *kTagUnderline = "u";
  static constexpr const char *kTagStrighthrought = "del";
  static constexpr const char *kTagStrighthrought2 = "s";

  static constexpr const char *kTagBlockquote = "blockquote";

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

  vector<string> dom_tags_;

  bool is_in_tag_ = false;
  bool is_closing_tag_ = false;
  bool is_in_attribute_value_ = false;
  bool is_in_pre_ = false;
  bool is_in_code_ = false;
  bool is_in_table_ = false;
  bool is_in_list_ = false;

  // relevant for <li> only, false = is in unordered list
  bool is_in_ordered_list_ = false;
  int index_li = 0;

  short index_blockquote = 0;

  char prev_ch_in_md_ = 0, prev_prev_ch_in_md_ = 0;
  char prev_ch_in_html_ = 'x';

  string html_;

  u_int16_t offset_lt_;
  string current_tag_;
  string prev_tag_;

  string current_attribute_;
  string current_attribute_value_;

  string current_href_;
  string current_title_;

  string current_alt_;
  string current_src_;

  // Line which separates header from data
  string tableLine;

  size_t chars_in_curr_line_ = 0;
  u_int16_t char_index_in_tag_content = 0;

  string md_;
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
                 ->AppendToMd(converter->current_href_);

        // If title is set append it
        if (!converter->current_title_.empty()) {
            converter->AppendToMd(" \"")
                     ->AppendToMd(converter->current_title_)
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
        if (converter->is_in_table_) {
          if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

          converter->AppendToMd("<br>");
        } else if (converter->md_len_ > 0) converter->AppendToMd("  \n");

        converter->AppendToMd(Repeat("> ", converter->index_blockquote));
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
      if (converter->prev_ch_in_md_ != '\n' &&
          converter->prev_prev_ch_in_md_ != '-') converter->AppendToMd("\n");

      if (!converter->is_in_ordered_list_) {
        converter->AppendToMd("- ");
        return;
      }

      ++converter->index_li;

      string num = to_string(converter->index_li);
      num += ". ";
      converter->AppendToMd(num);
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
      converter->is_in_list_ = true;
      converter->is_in_ordered_list_ = true;
      converter->index_li = 0;

      converter->ReplacePreviousSpaceInLineByNewline();

      converter->AppendToMd('\n');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      converter->is_in_list_ = false;
      converter->is_in_ordered_list_ = false;

      converter->AppendToMd('\n');
    }
  };

  struct TagParagraph : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      if (converter->is_in_list_ && converter->prev_tag_ == kTagParagraph)
        converter->AppendToMd("\n\t");
      else if (!converter->is_in_list_ && converter->index_blockquote == 0)
        converter->AppendToMd('\n');

      if (converter->index_blockquote > 0) {
        converter->AppendToMd("> \n");
        converter->AppendToMd(Repeat("> ", converter->index_blockquote));
      }
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

      if (converter->index_blockquote != 0) {
        converter->AppendToMd(Repeat("> ", converter->index_blockquote));
        converter->ShortenMarkdown();
      }

      if (converter->is_in_list_ && converter->prev_tag_ != kTagParagraph) converter->ShortenMarkdown(2);

      if (converter->is_in_list_ || converter->index_blockquote != 0)
        converter->AppendToMd("\t\t");
      else
        converter->AppendToMd("```");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      converter->is_in_pre_ = false;

      if (!converter->is_in_list_ && converter->index_blockquote == 0)
        converter->AppendToMd("```\n");
    }
  };

  struct TagCode : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      converter->is_in_code_ = true;

      if (converter->is_in_pre_) {
        // Remove space
        if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

        if (converter->is_in_list_ || converter->index_blockquote != 0) return;

        auto code = converter->ExtractAttributeFromTagLeftOf(kAttributeClass);
        if (!code.empty()) {
          code.erase(0, 9); // remove language-
          converter->AppendToMd(code);
        }
        converter->AppendToMd('\n');
      }
      else converter->AppendToMd('`');
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
      converter->is_in_list_ = true;

      if (converter->prev_prev_ch_in_md_ == '-' && converter->prev_ch_in_md_ == ' ') return;

      if (converter->prev_ch_in_md_ != '\n') converter->AppendToMd("\n");

      if (converter->prev_prev_ch_in_md_ != '\n') converter->AppendToMd("\n");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      converter->is_in_list_ = false;

      converter->AppendToMd('\n');
    }
  };

  struct TagImage : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          if (converter->prev_tag_ != kTagAnchor) converter->AppendToMd('\n');

          converter->AppendToMd("![")
                   ->AppendToMd(converter->ExtractAttributeFromTagLeftOf(kAttributeAlt))
                   ->AppendToMd("](")
                   ->AppendToMd(converter->ExtractAttributeFromTagLeftOf(kAttributeSrc))
                   ->AppendToMd(")");
      }
      void OnHasLeftClosingTag(Converter* converter) override {
          if (converter->prev_tag_ == kTagAnchor) converter->AppendToMd('\n');
      }
  };

  struct TagSeperator : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          converter->AppendToMd("\n---\n");
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
              converter->AppendToMd(converter->tableLine);
              converter->tableLine.clear();
          }
      }
  };

  struct TagTableHeader : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          auto align = converter->ExtractAttributeFromTagLeftOf(kAttrinuteAlign);

          string line = "| ";

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
      }
  };

  struct TagTableData : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          if (converter->prev_prev_ch_in_md_ != '|') converter->AppendToMd("| ");
      }
      void OnHasLeftClosingTag(Converter* converter) override {
      }
  };

  struct TagBlockquote : Tag {
    void OnHasLeftOpeningTag(Converter* converter) override {
      ++converter->index_blockquote;

      if (converter->index_blockquote == 1) converter->AppendToMd('\n');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      --converter->index_blockquote;
    }
  };

  map<string, Tag*> tags_;

  explicit Converter(string *html) : html_(*html) {
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

  // Trim from start (in place)
  static void LTrim(string *s) {
    (*s).erase(
        (*s).begin(),
        find_if(
            (*s).begin(),
            (*s).end(),
            not1(ptr_fun<int, int>(isspace))));
  }

  // Trim from end (in place)
  Converter * RTrim(string *s, bool trim_only_blank = false) {
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

  // Trim from both ends (in place)
  Converter * Trim(string *s) {
    if (!startsWith(*s, "\t"))
      LTrim(s);

    if (!endsWith(*s, "  "))
      RTrim(s);

    return this;
  }

  // 1. trim all lines
  // 2. reduce consecutive newlines to maximum 3
  void TidyAllLines(string *str) {
    auto lines = Explode(*str, '\n');
    string res;

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

      amount_replaced++;
    }

    return amount_replaced;
  }

  // Split given string by given character delimiter into vector of strings
  static vector<string> Explode(string const &str,
                                          char delimiter) {
    vector<string> result;
    istringstream iss(str);

    for (string token; getline(iss, token, delimiter);)
      result.push_back(move(token));

    return result;
  }

  // Repeat given amount of given string
  static string Repeat(const string &str, size_t amount) {
    if (amount == 1) return str;
    else if (amount == 0) return "";

    string out;

    for (u_int16_t i = 0; i < amount; i++) {
      out += str;
    }

    return out;
  }

  string ExtractAttributeFromTagLeftOf(const string& attr) {
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

    if (offset_opening_quote == string::npos) return "";

    offset_closing_quote = tag.find(wrapping_quote, offset_opening_quote + 1);

    if (offset_closing_quote == string::npos) return "";

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
  string Convert2Md() {
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

  static bool TagContainsAttributesToHide(string *tag) {
    return (*tag).find(" aria=\"hidden\"") != string::npos
        || (*tag).find("display:none") != string::npos
        || (*tag).find("visibility:hidden") != string::npos
        || (*tag).find("opacity:0") != string::npos
        || (*tag).find("Details-content--hidden-not-important")
            != string::npos;
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

  // Replace previous space (if any) in current markdown line by newline
  bool ReplacePreviousSpaceInLineByNewline() {
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

  static bool IsIgnoredTag(const string &tag) {
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
      string tag = dom_tags_[i];

      if (kTagPre == tag
          || kTagTitle == tag) return false;

      if (IsIgnoredTag(tag)) return true;
    }

    return false;
  }
};  // Converter

// Static wrapper for simple invocation
string Convert(string html) {
  return html2md::Converter::Convert(&html);
}

}  // namespace html2md
