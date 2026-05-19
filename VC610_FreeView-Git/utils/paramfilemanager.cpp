#include "paramfilemanager.h"


ParamFileManager::ParamFileManager()
{
    OutputFilePath = BASE_RESOURCE + QString("\\outputFile_param_arg.txt");
    BtFilePath = BASE_RESOURCE + QString("\\bullet_time_param_arg.txt");
    CamSettingFilePath = BASE_RESOURCE + QString("\\cameraSetting_arg.txt");
    AppSettingFilePath = BASE_RESOURCE + QString("\\AppSetting_arg.txt");
}

QJsonObject ParamFileManager::getObjectFromFile(QString fileName)
{
    QString path = BASE_RESOURCE + QString("\\" + fileName);
    QFile file(path);

    if (!file.exists())
    {
        writeToFile(fileName, "");
        return QJsonObject();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QJsonObject();

    QTextStream in(&file);
    QString jsonString = in.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    return doc.object();
}


QString ParamFileManager::Output_Param2Json(const QString dir, const QString prefix, const QString suffix)
{
    QJsonObject Output_Param_obj;

    Output_Param_obj["dir"] = dir;
    Output_Param_obj["prefix"] = prefix;
    Output_Param_obj["suffix"] = suffix;

    QJsonDocument json_doc(Output_Param_obj);

    return json_doc.toJson();
}

QStringList ParamFileManager::Json2Output_Param(const QString jsonString)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject Output_Param_obj = doc.object();

    QStringList result;

    result.append(Output_Param_obj["dir"].toString());
    result.append(Output_Param_obj["prefix"].toString());
    result.append(Output_Param_obj["suffix"].toString());

    return result;
}

QStringList ParamFileManager::getOutput_ParamFromFile()
{
    QFile file(OutputFilePath);

    if (!file.exists())
        createFile(OutputFilePath, Output_Param2Json("", "", ""));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // 打开文件失败，返回空字符串
        return QStringList();
    }

    QTextStream in(&file);
    QString JsonString = in.readAll();
    file.close();

    return Json2Output_Param(JsonString);
}

QString ParamFileManager::getFileString(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";

    QTextStream in(&file);
    return in.readAll();
}

std::string ParamFileManager::extractMainFileName(const std::string& path) {
    // 找到最后一个斜杠或反斜杠的位置
    size_t lastSlash = path.find_last_of("/\\");

    // 提取文件名部分（包括擴展名）
    std::string fileNameWithExtension = path.substr(lastSlash + 1);

    // 找到最后一个點的位置（擴展名的分隔符）
    size_t lastDot = fileNameWithExtension.find_last_of(".");

    // 提取主文件名部分（不包括擴展名）
    std::string mainFileName = fileNameWithExtension.substr(0, lastDot);

    return mainFileName;
}

QString ParamFileManager::getDefultTrajFile()
{
    return TrajectoryFilePath;
}



int ParamFileManager::checkResource()
{
    QDir dir(BASE_RESOURCE);

    if (!dir.exists())
    {
        if (dir.mkpath("."))
        {
//            bullet_time_param param;

//            QDir bt_dir(QString::fromStdString(param.file_name));
//            if(!bt_dir.exists())
//                param.file_name = QCoreApplication::applicationDirPath().toStdString();

//            createFile(ParamFileDir + "/" + template_bt_argFile, BT_Param2Json(param, 1));

//            createFile(BtFilePath, BT_Param2Json(param, 1));
        }
        else
        {
            return 1;
        }
    }
    else
    {
//        bullet_time_param param;
//        QDir bt_dir(QString::fromStdString(param.file_name));
//        if(!bt_dir.exists())
//            param.file_name = QCoreApplication::applicationDirPath().toStdString();

//        createFile(ParamFileDir + "/" + template_bt_argFile, BT_Param2Json(param, 1));

//        createFile(BtFilePath, BT_Param2Json(param, 1));
        createFile(OutputFilePath, Output_Param2Json("", "", ""));
    }

    return 0;
}

void ParamFileManager::createFile(QString path, QString context)
{
    QDir dir(BASE_RESOURCE);
    if(!dir.exists())
        dir.mkpath(".");

    QFile file(path);
    if (!file.exists()) // 檔案不存在
    {
        file.open(QIODevice::WriteOnly | QIODevice::Text);

        QTextStream stream(&file);
        stream << context << endl;
        file.close();
    }
}

void ParamFileManager::writeToFile(QString path, QString context)
{
    QDir dir(BASE_RESOURCE);
    if(!dir.exists())
        dir.mkpath(".");

    QFile file(BASE_RESOURCE + QString("\\" + path));
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream stream(&file);
    stream << context << endl;
    file.close();
}

void ParamFileManager::writeToFile(ParamFile paramfile, QString context)
{
    QDir dir(BASE_RESOURCE);
    if(!dir.exists())
        dir.mkpath(".");

    QString path;

    if(paramfile == ParamFile::BT)
        path = BtFilePath;

    if(paramfile == ParamFile::camSetting)
        path = CamSettingFilePath;

    if(paramfile == ParamFile::OutputFile)
        path = OutputFilePath;

    if(paramfile == ParamFile::AppSetting)
        path = AppSettingFilePath;

    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream stream(&file);
    stream << context << endl;
    file.close();
}

void ParamFileManager::writeToPath(QString path, QString context)
{
    QFile file(path);

    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream stream(&file);
    stream << context << endl;
    file.close();
}
