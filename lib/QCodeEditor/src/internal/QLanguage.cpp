// QCodeEditor
#include <QLanguage>

// Qt
#include <QIODevice>
#include <QXmlStreamReader>

QLanguage::QLanguage(QIODevice *device, QObject *parent)
  : QObject(parent)
  , m_loaded(false)
  , m_list()
{
  load(device);
}

bool QLanguage::load(QIODevice *device)
{
  if (device == nullptr)
  {
    return false;
  }

  QXmlStreamReader reader(device);

  QString name;
  QStringList list;
  bool readText = false;

  while (!reader.atEnd() && !reader.hasError())
  {
    auto type = reader.readNext();

    if (type == QXmlStreamReader::TokenType::StartElement)
    {
      if (reader.name() == "section")
      {
        if (!list.empty())
        {
          m_list[name] = list;
          list.clear();
        }

        name = reader.attributes().value("name").toString();
      }
      else if (reader.name() == "name")
      {
        readText = true;
      }
    }
    else if (type == QXmlStreamReader::TokenType::Characters && readText)
    {
      list << reader.text().toString();
      readText = false;
    }
  }

  if (!list.empty())
  {
    m_list[name] = list;
  }

  m_loaded = !reader.hasError();

  return m_loaded;
}

QStringList QLanguage::keys()
{
  return m_list.keys();
}

QStringList QLanguage::names(const QString &key)
{
  return m_list[key];
}

bool QLanguage::isLoaded() const
{
  return m_loaded;
}
