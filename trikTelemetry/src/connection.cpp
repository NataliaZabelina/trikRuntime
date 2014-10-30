#include "connection.h"

namespace trikTelemetry {

Connection::Connection(trikControl::Brick &brick)
	: trikKernel::Connection(trikKernel::Protocol::messageLength)
	, mBrick(brick)
{
}

void Connection::processData(QByteArray const &data)
{
	QString command(data);

	QString portsRequested("ports");
	QString dataRequested("data");
	QString singleSensorRequested("sensor:");
	QString accelerometerRequested("AccelerometerPort");
	QString gyroscopeRequested("GyroscopePort");

	QString answer;

	if (command.startsWith(dataRequested)) {
		answer = "data:";
		answer += "analog:";
		for (QString port : mBrick.sensorPorts(trikControl::Sensor::analogSensor)) {
			answer += port + "=" + QString::number(mBrick.sensor(port)->read()) + ":";
			answer += QString::number(mBrick.sensor(port)->readRawData()) + ",";
		}
		answer[answer.length() - 1] = ';';
		answer += "digital:";
		for (QString port : mBrick.sensorPorts(trikControl::Sensor::digitalSensor)) {
			answer += port + "=" + QString::number(mBrick.sensor(port)->read()) + ":";
			answer += QString::number(mBrick.sensor(port)->readRawData()) + ",";
		}
		answer[answer.length() - 1] = ';';
		answer += "special:";
		for (QString port : mBrick.sensorPorts(trikControl::Sensor::specialSensor)) {
			answer += port + "=" + QString::number(mBrick.sensor(port)->read()) + ":";
			answer += QString::number(mBrick.sensor(port)->readRawData()) + ",";
		}
		answer[answer.length() - 1] = ';';
		answer += "encoders:";
		for (QString port : mBrick.encoderPorts()) {
			answer += port + "=" + QString::number(mBrick.encoder(port)->read()) + ":";
			answer += QString::number(mBrick.encoder(port)->readRawData()) + ",";
		}
		answer[answer.length() - 1] = ';';
		answer += "accelerometer:" + serializeVector(mBrick.accelerometer()->read()) + ";";
		answer += "gyroscope:" + serializeVector(mBrick.gyroscope()->read());
	} else if (command.startsWith(portsRequested)) {
		answer = "ports:";
		answer += "analog:" + mBrick.sensorPorts(trikControl::Sensor::analogSensor).join(",") + ";";
		answer += "digital:" + mBrick.sensorPorts(trikControl::Sensor::digitalSensor).join(",") + ";";
		answer += "special:" + mBrick.sensorPorts(trikControl::Sensor::specialSensor).join(",") + ";";
		answer += "encoders:" + mBrick.encoderPorts().join(",");
	} else if (command.startsWith(singleSensorRequested)) {
		answer = command + ":";
		command.remove(0, singleSensorRequested.length());
		if (command.startsWith(accelerometerRequested)) {
			int dimension = command.at(command.length() - 1).toAscii() - 'X';
			answer += QString::number(mBrick.accelerometer()->read()[dimension]);
		} else if (command.startsWith(gyroscopeRequested)) {
			int dimension = command.at(command.length() - 1).toAscii() - 'X';
			answer += QString::number(mBrick.gyroscope()->read()[dimension]);
		} else if (mBrick.sensorPorts(trikControl::Sensor::analogSensor).contains(command)
				|| mBrick.sensorPorts(trikControl::Sensor::digitalSensor).contains(command)
				|| mBrick.sensorPorts(trikControl::Sensor::specialSensor).contains(command)){
			answer += QString::number(mBrick.sensor(command)->read());
		} else if (mBrick.encoderPorts().contains(command)) {
			answer += QString::number(mBrick.encoder(command)->read());
		}
	}

	send(answer.toUtf8());
}

QString Connection::serializeVector(QVector<int> const &vector) {
	QString result = "(";
	for (int coord : vector) {
		result += QString::number(coord) + ",";
	}

	result[result.length() - 1] = ')';
	return result;
}

}
