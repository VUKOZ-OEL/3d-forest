#ifndef JSTEXTAREA_H
#define JSTEXTAREA_H

#include <QWidget>
#include <QTextEdit>
#include <QSyntaxHighlighter>

class Highlighter;
class JSTextArea : public QTextEdit
{
    Q_OBJECT
public:
    explicit JSTextArea(QWidget *parent = 0);

signals:
    void executeJS(QString code);

public slots:
protected:
    virtual void keyPressEvent ( QKeyEvent * event );
private:
    Highlighter *highlighter;
};

// from Qt-Example:
class Highlighter : public QSyntaxHighlighter
  {
      Q_OBJECT

  public:
      Highlighter(QTextDocument *parent = 0);

  protected:
      void highlightBlock(const QString &text) override;

  private:
      struct HighlightingRule
      {
          QRegExp pattern;
          QTextCharFormat format;
      };
      QVector<HighlightingRule> highlightingRules;

      QRegExp commentStartExpression;
      QRegExp commentEndExpression;

      QTextCharFormat keywordFormat;
      QTextCharFormat classFormat;
      QTextCharFormat singleLineCommentFormat;
      QTextCharFormat multiLineCommentFormat;
      QTextCharFormat quotationFormat;
      QTextCharFormat singleQuotationFormat;
      QTextCharFormat functionFormat;
  };

#endif // JSTEXTAREA_H
