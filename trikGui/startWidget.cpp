/* Copyright 2013 - 2015 Roman Kurbatov and CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file was modified by Yurii Litvinov to make it comply with the requirements of trikRuntime
 * project. See git revision history for detailed changes. */

#include "startWidget.h"

#include <QtGui/QKeyEvent>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	#include <QtGui/QApplication>
#else
	#include <QtWidgets/QApplication>
#endif

#include <trikControl/motorInterface.h>

#include "fileManagerWidget.h"
#include "wiFiModeWidget.h"
#include "motorsWidget.h"
#include "sensorsSelectionWidget.h"
#include "communicationSettingsWidget.h"
#include "informationWidget.h"
#include "systemSettingsWidget.h"
#include "languageSelectionWidget.h"

using namespace trikGui;

using trikControl::MotorInterface;
using trikControl::SensorInterface;

StartWidget::StartWidget(Controller &controller, QWidget *parent)
	: MainWidget(parent)
	, mController(controller)
	, mFileManagerRoot(MainWidget::FileManagerRootType::scriptsDir)
	, mProgrammingWidget(controller)
{
	mTitleLabel.setText(tr("TRIK"));

	mMenuModel.appendRow(new QStandardItem(FileManagerWidget::menuEntry()));
	mMenuModel.appendRow(new QStandardItem(ProgrammingWidget::menuEntry()));

	QStandardItem * const settingsItem = new QStandardItem(tr("Settings"));
	mMenuModel.appendRow(settingsItem);

	QStandardItem * const testingItem = new QStandardItem(tr("Testing"));
	settingsItem->appendRow(testingItem);

	testingItem->appendRow(new QStandardItem(
			SensorsSelectionWidget::menuEntry(SensorsSelectionWidget::SensorType::analogSensor)));

	testingItem->appendRow(new QStandardItem(MotorsWidget::menuEntry(MotorInterface::Type::servoMotor)));
	testingItem->appendRow(new QStandardItem(MotorsWidget::menuEntry(MotorInterface::Type::powerMotor)));

	testingItem->appendRow(new QStandardItem(
			SensorsSelectionWidget::menuEntry(SensorsSelectionWidget::SensorType::digitalSensor)));

	testingItem->appendRow(new QStandardItem(
			SensorsSelectionWidget::menuEntry(SensorsSelectionWidget::SensorType::encoder)));

	QStandardItem * const networkItem = new QStandardItem(tr("Network and connectivity"));
	settingsItem->appendRow(networkItem);

	if (mController.mailbox()) {
		networkItem->appendRow(new QStandardItem(CommunicationSettingsWidget::menuEntry()));
	}

	networkItem->appendRow(new QStandardItem(WiFiModeWidget::menuEntry()));

	QStandardItem * const systemItem = new QStandardItem(tr("System"));
	settingsItem->appendRow(systemItem);
	systemItem->appendRow(new QStandardItem(SystemSettingsWidget::menuEntry()));
	systemItem->appendRow(new QStandardItem(InformationWidget::menuEntry()));

	mMenuModel.appendRow(new QStandardItem(LanguageSelectionWidget::menuEntry()));

	mMenuView.setModel(&mMenuModel);

	mMenuView.selectionModel()->select(
			mMenuModel.index(0, 0)
			, QItemSelectionModel::ClearAndSelect
			);

	mLayout.addWidget(&mTitleLabel);
	mLayout.addWidget(&mMenuView);

	setLayout(&mLayout);
}

StartWidget::~StartWidget()
{
}

void StartWidget::renewFocus()
{
	mMenuView.setFocus();
}

void StartWidget::launch()
{
	const QModelIndex &currentIndex = mMenuView.currentIndex();
	const QStandardItem * const currentItem = mMenuModel.itemFromIndex(currentIndex);
	if (currentItem->hasChildren()) {
		setRootIndex(currentIndex);
	} else {
		QString currentItemText = currentItem->text();

		int result = TrikGuiDialog::normalExit;

		if (currentItemText == FileManagerWidget::menuEntry()) {
			/// @todo Why widgets are created every time?
			FileManagerWidget fileManagerWidget(mController, mFileManagerRoot);
			emit newWidget(fileManagerWidget);
			result = fileManagerWidget.exec();
		} else if (currentItemText == WiFiModeWidget::menuEntry()) {
			WiFiModeWidget wiFiModeWidget(mController.wiFi());
			emit newWidget(wiFiModeWidget);
			result = wiFiModeWidget.exec();
		} else if (currentItemText == ProgrammingWidget::menuEntry()) {
			const auto keepProgramInMemoryWhenNotInProgrammingMode = false;
			if (keepProgramInMemoryWhenNotInProgrammingMode) {
				mProgrammingWidget.show();
			} else {
				ProgrammingWidget programmingWidget(mController);
				emit newWidget(programmingWidget);
				result = programmingWidget.exec();
			}

		} else if (currentItemText == MotorsWidget::menuEntry(MotorInterface::Type::powerMotor)) {
			MotorsWidget motorsWidget(mController.brick(), MotorInterface::Type::powerMotor);
			emit newWidget(motorsWidget);
			result = motorsWidget.exec();
		} else if (currentItemText == MotorsWidget::menuEntry(MotorInterface::Type::servoMotor)) {
			MotorsWidget motorsWidget(mController.brick(), MotorInterface::Type::servoMotor);
			emit newWidget(motorsWidget);
			result = motorsWidget.exec();

		} else if (currentItemText == SensorsSelectionWidget::menuEntry(
				SensorsSelectionWidget::SensorType::analogSensor))
		{
			SensorsSelectionWidget sensorsSelectionWidget(mController.brick()
					, SensorsSelectionWidget::SensorType::analogSensor);

			emit newWidget(sensorsSelectionWidget);
			result = sensorsSelectionWidget.exec();
		} else if (currentItemText == SensorsSelectionWidget::menuEntry(
				SensorsSelectionWidget::SensorType::digitalSensor))
		{
			SensorsSelectionWidget sensorsSelectionWidget(mController.brick()
					, SensorsSelectionWidget::SensorType::digitalSensor);

			emit newWidget(sensorsSelectionWidget);
			result = sensorsSelectionWidget.exec();
		} else if (currentItemText == SensorsSelectionWidget::menuEntry(
				SensorsSelectionWidget::SensorType::encoder))
		{
			SensorsSelectionWidget sensorsSelectionWidget(mController.brick()
					, SensorsSelectionWidget::SensorType::encoder);

			emit newWidget(sensorsSelectionWidget);
			result = sensorsSelectionWidget.exec();
		} else if (currentItemText == CommunicationSettingsWidget::menuEntry()) {
			if (mController.mailbox()) {
				CommunicationSettingsWidget communicationSettingsWidget(*mController.mailbox());
				emit newWidget(communicationSettingsWidget);
				result = communicationSettingsWidget.exec();
			} else {
				Q_ASSERT(!"Mailbox is disabled but commmunications widget still tries to be shown");
			}
		} else if (currentItemText == InformationWidget::menuEntry()) {
			InformationWidget versionWidget;
			emit newWidget(versionWidget);
			result = versionWidget.exec();
		} else if (currentItemText == SystemSettingsWidget::menuEntry()) {
			SystemSettingsWidget systemSettingsWidget(mFileManagerRoot);
			connect(&systemSettingsWidget, SIGNAL(currentFilesDirPath(MainWidget::FileManagerRootType const&))
					, this, SLOT(changeFileManagerRoot(MainWidget::FileManagerRootType const&)));

			emit newWidget(systemSettingsWidget);
			result = systemSettingsWidget.exec();
		} else if (currentItemText == LanguageSelectionWidget::menuEntry()) {
			LanguageSelectionWidget languageSelectionWidget;
			emit newWidget(languageSelectionWidget);
			result = languageSelectionWidget.exec();
		}

		if (result == TrikGuiDialog::goHomeExit) {
			goHome();
		}
	}
}

void StartWidget::changeFileManagerRoot(MainWidget::FileManagerRootType const& path)
{
	mFileManagerRoot = path;
}

void StartWidget::setRootIndex(const QModelIndex &index)
{
	const QStandardItem *item = mMenuModel.itemFromIndex(index);

	if (item == nullptr) {
		item = mMenuModel.invisibleRootItem();
	}

	if (!item->hasChildren()) {
		return;
	}

	mSelections[mMenuView.rootIndex()] = mMenuView.currentIndex();

	mMenuView.setRootIndex(index);

	QModelIndex selectedItemIndex;
	if (mSelections.contains(index)) {
		selectedItemIndex = mSelections[index];
	} else {
		selectedItemIndex = mMenuModel.indexFromItem(item->child(0));
	}

	mMenuView.selectionModel()->select(
			selectedItemIndex
			, QItemSelectionModel::ClearAndSelect
			);

	mMenuView.setCurrentIndex(selectedItemIndex);
}

void StartWidget::goHome()
{
	setRootIndex(QModelIndex());
}

void StartWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
		case Qt::Key_PowerDown: {
			goHome();
			break;
		}
		case Qt::Key_Escape: {
			QStandardItem const * const rootItem = mMenuModel.itemFromIndex(mMenuView.rootIndex());
			if (rootItem == nullptr) {
				break;
			}

			setRootIndex(mMenuModel.indexFromItem(rootItem->parent()));
			break;
		}
		case Qt::Key_Return: {
			launch();
			break;
		}
		default: {
			MainWidget::keyPressEvent(event);
			break;
		}
	}
}

