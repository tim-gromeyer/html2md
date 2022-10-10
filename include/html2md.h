// Copyright (c) Tim Gromeyer
// Licensed under the MIT License - https://opensource.org/licenses/MIT

#ifndef HTML2MD_H
#define HTML2MD_H

#include <map>
#include <string>
#include <vector>


namespace html2md {

namespace utils {

static bool startsWith(const std::string& str, const std::string& prefix);

static bool endsWith(const std::string& str, const std::string& suffix);

static int ReplaceAll(std::string *haystack,
                      const std::string &needle,
                      const std::string &replacement);

// Split given std::string by given character delimiter into vector of std::strings
static std::vector<std::string> Explode(std::string const &str, char delimiter);

// Repeat given amount of given std::string
static std::string Repeat(const std::string &str, size_t amount) {
  if (amount == 1) return str;
  else if (amount == 0) return "";

  std::string out;

  for (u_int16_t i = 0; i < amount; i++) {
    out += str;
  }

  return out;
}
}

// Main class: HTML to Markdown converter
class Converter {
 public:
  explicit inline Converter(std::string &html) {
      *this = Converter(&html);
  }

  ~Converter() = default;

  // Main character iteration of parser
  std::string Convert2Md();

  void PrepareHtml();

  void CleanUpMarkdown();

  Converter* AppendToMd(char ch);

  Converter* AppendToMd(const char *str);

  inline Converter* AppendToMd(const std::string &s) {
      return AppendToMd(s.c_str());
  }

  // Append ' ' if:
  // - md does not end w/ '**'
  // - md does not end w/ '\n'
  Converter* AppendBlank();

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

  std::vector<std::string> dom_tags_;

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
      if (converter->prev_prev_ch_in_md_ == ' ' &&
          converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

      converter->AppendToMd("<u>");
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->prev_ch_in_md_ == ' ') converter->ShortenMarkdown();

      converter->AppendToMd("</u>");
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

        converter->AppendToMd(utils::Repeat("> ", converter->index_blockquote));
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
      // if (converter->prev_ch_in_md_ != '\n' &&
      //     converter->prev_prev_ch_in_md_ != '-') converter->AppendToMd("\n");
      if (converter->is_in_table_) return;

      if (!converter->is_in_ordered_list_) {
        converter->AppendToMd("- ");
        return;
      }

      ++converter->index_li;

      std::string num = std::to_string(converter->index_li);
      num += ". ";
      converter->AppendToMd(num);
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->is_in_table_) return;

      if (converter->prev_ch_in_md_ != '\n') converter->AppendToMd('\n');
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
      if (converter->is_in_table_) return;

      converter->is_in_list_ = true;
      converter->is_in_ordered_list_ = true;
      converter->index_li = 0;

      converter->ReplacePreviousSpaceInLineByNewline();

      converter->AppendToMd('\n');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->is_in_table_) return;

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
        converter->AppendToMd(utils::Repeat("> ", converter->index_blockquote));
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
        converter->AppendToMd(utils::Repeat("> ", converter->index_blockquote));
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
      if (converter->is_in_list_ ||
          converter->is_in_table_) return;

      converter->is_in_list_ = true;

      converter->AppendToMd('\n');
    }
    void OnHasLeftClosingTag(Converter* converter) override {
      if (converter->is_in_table_) return;

      converter->is_in_list_ = false;

      if ((converter->prev_prev_ch_in_md_ == '*' ||
           converter->prev_prev_ch_in_md_ == '-' ||
           converter->prev_prev_ch_in_md_ == '+' ||
           converter->prev_prev_ch_in_md_ == '.' ||
           converter->prev_prev_ch_in_md_ == ')') &&
           converter->prev_tag_ != kTagParagraph)
        converter->is_in_list_ = true;

      if (converter->prev_prev_ch_in_md_ == '\n' &&
          converter->prev_ch_in_md_ == '\n') converter->ShortenMarkdown();
      else if (converter->prev_ch_in_md_ != '\n') converter->AppendToMd('\n');
    }
  };

  struct TagImage : Tag {
      void OnHasLeftOpeningTag(Converter* converter) override {
          if (converter->prev_tag_ != kTagAnchor &&
              converter->prev_ch_in_md_ != '\n') converter->AppendToMd('\n');

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
          converter->UpdatePrevChFromMd();
          if (converter->prev_ch_in_md_ == '|') converter->AppendToMd('\n'); // There's a bug
          else converter->AppendToMd('|');

          if (!converter->tableLine.empty()) {
              if (converter->prev_ch_in_md_ != '\n') converter->AppendToMd('\n');

              converter->tableLine.append("|\n");
              converter->AppendToMd(converter->tableLine);
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

  std::map<std::string, Tag*> tags_;

  explicit Converter(std::string *html);

  // Trim from start (in place)
  static void LTrim(std::string *s);

  // Trim from end (in place)
  Converter* RTrim(std::string *s, bool trim_only_blank = false);

  // Trim from both ends (in place)
  Converter * Trim(std::string *s);

  // 1. trim all lines
  // 2. reduce consecutive newlines to maximum 3
  void TidyAllLines(std::string *str);

  std::string ExtractAttributeFromTagLeftOf(const std::string& attr);

  void TurnLineIntoHeader1();

  void TurnLineIntoHeader2();

  inline void UpdateMdLen() { md_len_ = md_.length(); }

  // Current char: '<'
  void OnHasEnteredTag();

  Converter* UpdatePrevChFromMd();

  /**
   * Handle next char within <...> tag
   *
   * @param ch current character
   * @return   continue surrounding iteration?
   */
  bool ParseCharInTag(char ch);

  // Current char: '>'
  bool OnHasLeftTag();

  inline static bool TagContainsAttributesToHide(std::string *tag) {
    return (*tag).find(" aria=\"hidden\"") != std::string::npos
        || (*tag).find("display:none") != std::string::npos
        || (*tag).find("visibility:hidden") != std::string::npos
        || (*tag).find("opacity:0") != std::string::npos
        || (*tag).find("Details-content--hidden-not-important")
            != std::string::npos;
  }

  Converter* ShortenMarkdown(int chars = 1);

  /**
   * @param ch
   * @return continue iteration surrounding  this method's invocation?
   */
  bool ParseCharInTagContent(char ch);

  // Replace previous space (if any) in current markdown line by newline
  bool ReplacePreviousSpaceInLineByNewline();

  inline static bool IsIgnoredTag(const std::string &tag) {
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
inline std::string Convert(std::string &html) {
  Converter c(html);
  return c.Convert2Md();
}

}  // namespace html2md

#endif // HTML2MD_H
