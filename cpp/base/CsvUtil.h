
#ifndef CSV_UTIL_H_
#define CSV_UTIL_H_

#include <iostream>
#include "MyJsonTools.hpp"



extern void readFromCsv(std::istream& f, std::shared_ptr<Wt::WAbstractItemModel> model,
                        int numRows = -1, bool firstLineIsHeaders = true,std::string table="empty");

extern void readFromCsv(std::istream& f, Wt::WAbstractItemModel *model,
                        int numRows = -1, bool firstLineIsHeaders = true,std::string table="empty");


extern std::shared_ptr<Wt::WStandardItemModel> csvToModel(const std::string& csvFile,
                                          bool firstLineIsHeader = true,std::string table="empty");

extern std::shared_ptr<Wt::WStandardItemModel> csvModel(std::string& csvdata,
                                          bool firstLineIsHeader = true,std::string table="empty");

#endif // CSV_UTIL_H_
