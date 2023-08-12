// ***************************************************************************************
// Copyright (c) 2023-2025 Peng Cheng Laboratory
// Copyright (c) 2023-2025 Institute of Computing Technology, Chinese Academy of Sciences
// Copyright (c) 2023-2025 Beijing Institute of Open Source Chip
//
// iEDA is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
// http://license.coscl.org.cn/MulanPSL2
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
//
// See the Mulan PSL v2 for more details.
// ***************************************************************************************
#ifndef GUISPLASH_H
#define GUISPLASH_H

#include <QLabel>
#include <QSplashScreen>

class GuiSplash : public QSplashScreen {
  Q_OBJECT
 public:
  explicit GuiSplash(QWidget *parent = nullptr,
                     const QPixmap &pixmap = QPixmap(),
                     Qt::WindowFlags f = Qt::WindowFlags());
  void loadingSlot(const QString &text);

 signals:
  void loading(const QString &text);

 private:
  QLabel *load;
  void setlabel(const QString &text);
};

#endif  // GUISPLASH_H