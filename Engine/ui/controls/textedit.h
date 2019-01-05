#pragma once

#include <Tempest/TextModel>
#include <Tempest/Widget>

namespace Tempest {

class TextEdit : public Tempest::Widget {
  public:
    TextEdit();

    void setText(const char* text);
    void setFont(const Font& f);

  protected:
    void paintEvent(Tempest::PaintEvent& e) override;

  private:
    TextModel textM;

    void      invalidateSizeHint();
  };

}