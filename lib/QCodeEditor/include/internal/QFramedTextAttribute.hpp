#pragma once

// Qt
#include <QObject>              // Required for inheritance
#include <QTextObjectInterface> // Required for inheritance

class QSyntaxStyle;

/**
 * @brief Class, that describes
 * attribute for making text frame.
 */
class QFramedTextAttribute : public QObject, public QTextObjectInterface
{
  Q_OBJECT
  Q_INTERFACES(QTextObjectInterface)

public:
  enum Property
  {
    FramedString = 1
  };

  /**
   * @brief Static method for getting framed text
   * attribute type.
   */
  static int type();

  /**
   * @brief Constructor.
   * @param parent Pointer to parent QObject.
   */
  explicit QFramedTextAttribute(QObject *parent = nullptr);

  // Disable copying
  QFramedTextAttribute(const QFramedTextAttribute &) = delete;
  QFramedTextAttribute &operator=(const QFramedTextAttribute &) = delete;

  /**
   * @brief Method for getting custom element (frame)
   * size. Though frame symbol has no size, this
   * method returns {0, 0}
   */
  QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                       const QTextFormat &format) override;

  /**
   * @brief Method for drawing frame.
   */
  void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                  int posInDocument, const QTextFormat &format) override;

  /**
   * @brief Method for creating frame in cursor
   * selection.
   * @param cursor Cursor.
   */
  void frame(QTextCursor cursor);

  /**
   * @brief Method for clearing all frames
   * with desired cursor.
   */
  void clear(QTextCursor cursor);

  /**
   * @brief Method for setting syntax style
   * for rendering.
   */
  void setSyntaxStyle(QSyntaxStyle *style);

  /**
   * @brief Method for getting syntax style.
   */
  QSyntaxStyle *syntaxStyle() const;

private:
  QSyntaxStyle *m_style;
};
