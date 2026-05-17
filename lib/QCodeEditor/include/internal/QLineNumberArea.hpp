#pragma once

// Qt
#include <QWidget> // Required for inheritance

class QCodeEditor;
class QSyntaxStyle;

/**
 * @brief Class, that describes line number area widget.
 */
class QLineNumberArea : public QWidget
{
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent Pointer to parent QTextEdit widget.
   */
  explicit QLineNumberArea(QCodeEditor *parent = nullptr);

  // Disable copying
  QLineNumberArea(const QLineNumberArea &) = delete;
  QLineNumberArea &operator=(const QLineNumberArea &) = delete;

  /**
   * @brief Overridden method for getting line number area
   * size.
   */
  QSize sizeHint() const override;

  /**
   * @brief Method for setting syntax style object.
   * @param style Pointer to syntax style.
   */
  void setSyntaxStyle(QSyntaxStyle *style);

  /**
   * @brief Method for getting syntax style.
   * @return Pointer to syntax style.
   */
  QSyntaxStyle *syntaxStyle() const;

protected:
  void paintEvent(QPaintEvent *event) override;

  /**
   * @brief Intercepts LayoutDirectionChange events to keep the gutter
   * pinned to left-to-right regardless of the host application's locale.
   * Line numbers and the gutter geometry are always rendered LTR so they
   * stay aligned with the editor's LTR text flow.
   */
  void changeEvent(QEvent *event) override;

private:
  QSyntaxStyle *m_syntaxStyle;

  QCodeEditor *m_codeEditParent;
};
