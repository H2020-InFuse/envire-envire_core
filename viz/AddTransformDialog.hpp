#pragma once
#include <QDialog>
#include "ui_addtransformdialog.h"
#include "TransformModel.hpp"
#include <base/TransformWithCovariance.hpp>

namespace envire { namespace viz {
  
class AddTransformDialog : public QDialog
{
  Q_OBJECT
public:
  AddTransformDialog(QWidget* parent = nullptr);
  QString getFrameId() const;
  const base::TransformWithCovariance& getTransform() const;
  
private:
  Ui::AddTransformDialog window;
  TransformModel model;
};
}}