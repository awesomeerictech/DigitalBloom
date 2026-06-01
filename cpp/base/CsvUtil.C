#include <fstream>

#include <boost/tokenizer.hpp>

#include <Wt/WAbstractItemModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WString.h>

#include "CsvUtil.h"

/*
 * A standard item which converts text edits to numbers
 */
class NumericItem : public Wt::WStandardItem {
public:
  virtual std::unique_ptr<WStandardItem> clone() const {
    return std::unique_ptr<NumericItem>(std::make_unique<NumericItem>());
  }

  virtual void setData(const Wt::cpp17::any &data, Wt::ItemDataRole role = Wt::ItemDataRole::User) {
    Wt::cpp17::any dt;

    if (role == Wt::ItemDataRole::Edit) {
      std::string s = Wt::asString(data).toUTF8();

      char *end;
      double d = std::strtod(s.c_str(), &end);
      if (*end == 0)
        dt = Wt::cpp17::any(d);
      else
        dt = data;
    } else
      dt = data;

    WStandardItem::setData(dt, role);
  }
};

class UserItem : public Wt::WStandardItem {
public:
  virtual std::unique_ptr<WStandardItem> clone() const {
    return std::unique_ptr<UserItem>(std::make_unique<UserItem>());
  }

    const Wt::ItemDataRole Role1 = Wt::ItemDataRole::User;
    const Wt::ItemDataRole Role2 = Wt::ItemDataRole::User + 1;
    const Wt::ItemDataRole Role3 = Wt::ItemDataRole::User + 2;
    const Wt::ItemDataRole Role4 = Wt::ItemDataRole::User+3;
    const Wt::ItemDataRole Role5 = Wt::ItemDataRole::User + 4;
    const Wt::ItemDataRole Role6 = Wt::ItemDataRole::User + 5;
    const Wt::ItemDataRole Role7 = Wt::ItemDataRole::User + 6;
    const Wt::ItemDataRole Role8 = Wt::ItemDataRole::User + 7;
    const Wt::ItemDataRole Role9 = Wt::ItemDataRole::User + 8;
    const Wt::ItemDataRole Role10 = Wt::ItemDataRole::User + 9;
    const Wt::ItemDataRole Role11 = Wt::ItemDataRole::User + 10;
    const Wt::ItemDataRole Role12 = Wt::ItemDataRole::User + 11;
    const Wt::ItemDataRole Role13 = Wt::ItemDataRole::User + 12;
    const Wt::ItemDataRole Role14 = Wt::ItemDataRole::User + 13;
    const Wt::ItemDataRole Role15 = Wt::ItemDataRole::User + 14;
    const Wt::ItemDataRole Role16 = Wt::ItemDataRole::User + 15;
    const Wt::ItemDataRole Role17 = Wt::ItemDataRole::User + 16;
    const Wt::ItemDataRole Role18 = Wt::ItemDataRole::User + 17;
    const Wt::ItemDataRole Role19 = Wt::ItemDataRole::User + 18;
    const Wt::ItemDataRole Role20 = Wt::ItemDataRole::User + 19;
    const Wt::ItemDataRole Role21 = Wt::ItemDataRole::User + 20;
    const Wt::ItemDataRole Role22 = Wt::ItemDataRole::User + 21;
    const Wt::ItemDataRole Role23 = Wt::ItemDataRole::User + 22;
    const Wt::ItemDataRole Role24 = Wt::ItemDataRole::User + 23;
    const Wt::ItemDataRole Role25 = Wt::ItemDataRole::User + 24;
    const Wt::ItemDataRole Role26 = Wt::ItemDataRole::User + 25;
    const Wt::ItemDataRole Role27 = Wt::ItemDataRole::User + 26;
    const Wt::ItemDataRole Role28 = Wt::ItemDataRole::User + 27;
    const Wt::ItemDataRole Role29 = Wt::ItemDataRole::User + 28;
    const Wt::ItemDataRole Role30 = Wt::ItemDataRole::User + 29;
    const Wt::ItemDataRole Role31 = Wt::ItemDataRole::User + 30;
    const Wt::ItemDataRole Role32 = Wt::ItemDataRole::User + 31;
    const Wt::ItemDataRole Role33 = Wt::ItemDataRole::User + 32;
    const Wt::ItemDataRole Role34 = Wt::ItemDataRole::User + 33;
    const Wt::ItemDataRole Role35 = Wt::ItemDataRole::User + 34;
    const Wt::ItemDataRole Role36 = Wt::ItemDataRole::User + 35;
    const Wt::ItemDataRole Role37 = Wt::ItemDataRole::User + 36;
    const Wt::ItemDataRole Role38 = Wt::ItemDataRole::User + 37;
    const Wt::ItemDataRole Role39 = Wt::ItemDataRole::User + 38;
    const Wt::ItemDataRole Role40 = Wt::ItemDataRole::User + 39;
    const Wt::ItemDataRole Role41 = Wt::ItemDataRole::User + 40;
    const Wt::ItemDataRole Role42 = Wt::ItemDataRole::User + 41;
    const Wt::ItemDataRole Role43 = Wt::ItemDataRole::User + 42;
    const Wt::ItemDataRole Role44 = Wt::ItemDataRole::User + 43;
    const Wt::ItemDataRole Role45 = Wt::ItemDataRole::User + 44;
    const Wt::ItemDataRole Role46 = Wt::ItemDataRole::User + 45;
    const Wt::ItemDataRole Role47 = Wt::ItemDataRole::User + 46;
    const Wt::ItemDataRole Role48 = Wt::ItemDataRole::User + 47;
    const Wt::ItemDataRole Role49 = Wt::ItemDataRole::User + 48;
    const Wt::ItemDataRole Role50 = Wt::ItemDataRole::User + 49;


  virtual void setData(const Wt::cpp17::any &data, Wt::ItemDataRole role = Wt::ItemDataRole::User) {
    Wt::cpp17::any dt;

    if (role == Role1) {

    }

    else if(role == Role2) {


    }

    else if(role == Role3) {


    }
    else if(role == Role4) {


    }
    else if(role == Role5) {


    }
    else if(role == Role6) {


    }
    else if(role == Role7) {


    }
    else if(role == Role8) {


    }
    else if(role == Role9) {


    }
    else if(role == Role10) {


    }
    else if(role == Role11) {


    }
    else if(role == Role12) {


    }
    else if(role == Role13) {


    }
    else if(role == Role14) {


    }
    else if(role == Role15) {


    }
    else if(role == Role16) {


    }
    else if(role == Role17) {


    }
    else if(role == Role18) {


    }
    else if(role == Role19) {


    }
    else if(role == Role20) {


    }
    else if(role == Role21) {


    }
    else if(role == Role22) {


    }
    else if(role == Role23) {


    }
    else if(role == Role24) {


    }
    else if(role == Role25) {


    }
    else if(role == Role26) {


    }
    else if(role == Role27) {


    }
    else if(role == Role28) {


    }
    else if(role == Role29) {


    }
    else if(role == Role30) {


    }
    else if(role == Role31) {


    }
    else if(role == Role32) {


    }
    else if(role == Role33) {


    }
    else if(role == Role34) {


    }
    else if(role == Role35) {


    }
    else if(role == Role36) {


    }
    else if(role == Role37) {


    }
    else if(role == Role38) {


    }
    else if(role == Role39) {


    }
    else if(role == Role40) {


    }
    else if(role == Role41) {


    }
    else if(role == Role42) {


    }
    else if(role == Role43) {


    }
    else if(role == Role44) {


    }
    else if(role == Role45) {


    }
    else if(role == Role46) {


    }
    else if(role == Role47) {


    }
    else if(role == Role48) {


    }
    else if(role == Role49) {


    }
    else if(role == Role50) {


    }

    WStandardItem::setData(dt, role);
  }
};

void readFromCsv(std::istream& f, Wt::WAbstractItemModel *model,
                 int numRows, bool firstLineIsHeaders,std::string table)
{
  int csvRow = 0;

  while (f) {
    std::string line;
    getline(f, line);

    if (f) {
      typedef boost::tokenizer<boost::escaped_list_separator<char> >
        CsvTokenizer;
      CsvTokenizer tok(line);

      int col = 0;
      for (CsvTokenizer::iterator i = tok.begin();
           i != tok.end(); ++i, ++col) {

        if (col >= model->columnCount())
          model->insertColumns(model->columnCount(),
                               col + 1 - model->columnCount());

        if (firstLineIsHeaders && csvRow == 0)
          model->setHeaderData(col, Wt::cpp17::any(Wt::WString(*i)));
        else {
          int dataRow = firstLineIsHeaders ? csvRow - 1 : csvRow;

          if (numRows != -1 && dataRow >= numRows)
            return;

          if (dataRow >= model->rowCount())
            model->insertRows(model->rowCount(),
                              dataRow + 1 - model->rowCount());

          Wt::cpp17::any data;
          std::string s = *i;

          char *endptr;

          if (s.empty())
            data = Wt::cpp17::any();
          else {
            double d = strtod(s.c_str(), &endptr);
            if (*endptr == 0)
              data = Wt::cpp17::any(d);
            else
              data = Wt::cpp17::any(Wt::WString(s));
          }

          model->setData(dataRow, col, data);
        }
      }
    }

    ++csvRow;
  }
}
std::shared_ptr<Wt::WStandardItemModel> csvToModel(const std::string& csvFile,
                                   bool firstLineIsHeaders,std::string table)
{
  std::ifstream f(csvFile.c_str());

  if (f) {
    std::shared_ptr<Wt::WStandardItemModel> result = std::make_shared<Wt::WStandardItemModel>(0, 0);
    result->setItemPrototype(std::make_unique<NumericItem>());
    readFromCsv(f, result, -1, firstLineIsHeaders);
    return result;
  } else
    return nullptr;
}

std::shared_ptr<Wt::WStandardItemModel> csvModel(std::string& csvdata,
                                             bool firstLineIsHeader,std::string table ) {


    std::istringstream istr(csvdata);
    if (istr) {
      std::shared_ptr<Wt::WStandardItemModel> result = std::make_shared<Wt::WStandardItemModel>(0, 0);
      readFromCsv(istr, result, -1, firstLineIsHeader);
      return result;
    } else
      return nullptr;

}

void readFromCsv(std::istream& f, std::shared_ptr<Wt::WAbstractItemModel> model,
                 int numRows, bool firstLineIsHeaders,std::string table)
{
  int csvRow = 0;

  while (f) {
    std::string line;
    getline(f, line);

    if (f) {
      typedef boost::tokenizer<boost::escaped_list_separator<char> >
        CsvTokenizer;
      CsvTokenizer tok(line);

      int col = 0;
      for (CsvTokenizer::iterator i = tok.begin();
           i != tok.end(); ++i, ++col) {

        if (col >= model->columnCount())
          model->insertColumns(model->columnCount(),
                               col + 1 - model->columnCount());

        if (firstLineIsHeaders && csvRow == 0)
          model->setHeaderData(col, Wt::cpp17::any{Wt::WString{*i}});
        else {
          int dataRow = firstLineIsHeaders ? csvRow - 1 : csvRow;

          if (numRows != -1 && dataRow >= numRows)
            return;

          if (dataRow >= model->rowCount())
            model->insertRows(model->rowCount(),
                              dataRow + 1 - model->rowCount());

          Wt::cpp17::any data{Wt::WString{*i}};
          model->setData(dataRow, col, data);
        }
      }
    }

    ++csvRow;
  }
}
