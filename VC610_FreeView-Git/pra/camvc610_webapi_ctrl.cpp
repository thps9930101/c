#include "camvc610_webapi_ctrl.h"

QStringList getAllNetCard(){
    // 获取所有网络接口
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    QStringList NetCardList;

    foreach (QNetworkInterface _interface, interfaces) {
        // 获取网络接口的名称
        QString interfaceName = _interface.humanReadableName();

        // 获取网络接口的IP地址列表
        QList<QNetworkAddressEntry> entries = _interface.addressEntries();

        // 遍历每个IP地址
        foreach (QNetworkAddressEntry entry, entries) {
            // 获取IP地址
            QHostAddress ipAddress = entry.ip();

            // 检查IP地址类型是否为IPv4
            if (ipAddress.protocol() == QAbstractSocket::IPv4Protocol) {
                // 输出网络接口名称和IPv4地址
                QString ip = ipAddress.toString();
                QString _ip = ip.left(ip.lastIndexOf(".") + 1);
                qDebug() << "Interface:" << interfaceName << "IPv4 Address:" << ip;
                if (!NetCardList.contains(_ip))
                    NetCardList.append(_ip);
            }
        }
    }
    return NetCardList;
}

//==========================
CamVC610_WebAPI_Ctrl::CamVC610_WebAPI_Ctrl()
{
//    curl_global_init(CURL_GLOBAL_ALL);
    connect(this, &CamVC610_WebAPI_Ctrl::setListFinish, this, &CamVC610_WebAPI_Ctrl::setListDateTimeFinish);
}


void CamVC610_WebAPI_Ctrl::setListDatetime(QVector<CamInfo> camIPList)
{
    std::thread([=](){
        std::string time = getCurrentDateTime().toStdString();
        for (int i = 0; i< camIPList.size(); i++)
        {
            QString _ip = camIPList[i].ip;

            CamVC610_WebAPI_Ctrl *wabAPI = new CamVC610_WebAPI_Ctrl();
            wabAPI->setIP(_ip);

            //!!!!
            //qDebug()<<"NoTimeSet";
            std::string json = "{\"name\":\"setOption\",\"parameters\":{\"options\":{\"dateTime\":\"" + time + "\"}}}";
            std::string result = send_url_API(_ip, "execute", json);
            int id = getStatusID(result);
            if(id < 0)
            {
                continue;
            }

            std::string json_check = "{\"id\":" + std::to_string(id) + "}";
            while(!isOperationDone(send_url_API(_ip, "status", json_check)))
            {
                Sleep(1000);
                qDebug()<<"...";
            }
            qDebug() << _ip << ": Set OK";
        }
        emit setListFinish();
    }).detach();
}

int CamVC610_WebAPI_Ctrl::setDatetime()
{
//!!!!
    std::string json = "{\"name\":\"setOption\",\"parameters\":{\"options\":{\"dateTime\":\"" + getCurrentDateTime().toStdString() + "\"}}}";
    std::string result = send_url_API(ip, "execute", json);
    return getStatusID(result);
    return 0;
}


int CamVC610_WebAPI_Ctrl::checkState(QString ip)
{
    return isStreaming(send_url_API(ip, "state", ""));
}

int CamVC610_WebAPI_Ctrl::checkStatus(int StatusID)
{
    std::string json = "{\"id\":" + std::to_string(StatusID) + "}";
    return isOperationDone(send_url_API(ip, "status", json));
}

int CamVC610_WebAPI_Ctrl::settingParam1(QString key, QString value)
{
    //POST Data
    std::string json = "{\"name\":\"setOption\",\"parameters\":{\"options\":{\""+key.toStdString()+"\":\""+value.toStdString()+"\"}}}";

    return isOperationDone(send_url_API(ip, "execute", json));
}

int CamVC610_WebAPI_Ctrl::settingParam2(QString key, int value)
{
    //POST Data
    std::string json = "{\"name\":\"setOption\",\"parameters\":{\"options\":{\""+key.toStdString()+"\":"+std::to_string(value)+"}}}";
    return isOperationDone(send_url_API(ip, "execute", json));
}

Cam_Set CamVC610_WebAPI_Ctrl::getParam()
{
    Cam_Set set;
    //POST Data
    std::string json = "{\"name\": \"getOption\",\"parameters\": {\"optionNames\": [ \"all\" ]}}";

    std::string resultString = send_url_API(ip, "execute", json);

    resultString.erase(std::remove(resultString.begin(), resultString.end(), '\n'), resultString.end());
    QByteArray jsonData(resultString.c_str(), resultString.length());

    QJsonObject jsonDoc = QJsonDocument::fromJson(jsonData)["results"]["options"].toObject();
    qDebug().noquote() << QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact);

    // video
    set.videoResolution = jsonDoc["videoResolution"].toString();
    set.videoFrameRate  = jsonDoc["videoFrameRate"].toString();
    set.videoGop        = jsonDoc["videoGop"].toInt();
    set.videoBitRate    = jsonDoc["videoBitRate"].toString();

    // Audio
    set.micSelect    = QString::number(jsonDoc["micSelect"].toDouble());
    qDebug()<<"getParam=======================";
    qDebug()<<"set.micSelect："<<set.micSelect;
    // image
    set.exposureTime    = jsonDoc["exposureTime"].toString();
    set.iso             = jsonDoc["iso"].toString();
    set.ev              = jsonDoc["ev"].toString();
    set.ae              = jsonDoc["ae"].toString();
    set.wdr             = jsonDoc["wdr"].toString();
    set.wb              = jsonDoc["wb"].toString();
    set.wbCT            = jsonDoc["wbCT"].toInt();

    return set;
}

void CamVC610_WebAPI_Ctrl::autoScanCamIP()
{
    QStringList ipBase_List = getAllNetCard();
    isScan = true;
    count = 0;
    countMax = ipBase_List.size() * 254;
    Camlist.clear();

    int start = 1;
    int end = 254;

    for (int i = start; i <= end; i++)
    {
        Sleep(1);
        std::thread([=]()
        {
            for (auto ipBase : ipBase_List){

                QString temp_ip = ipBase + QString::number(i);
                std::string res = send_url_API(temp_ip, "info", "");
                QString result = QString::fromStdString(res);
                mtx.lock();

                if(result.contains("slave not support"))
                {
        //            qDebug() << "slave";
                }
                else if(result.contains("IP") && result.contains("VC610"))
                {
        //            qDebug() << "master";
                    res.erase(std::remove(res.begin(), res.end(), '\n'), res.end());
                    QByteArray jsonData(res.c_str(), res.length());
                    QJsonObject jsonObj = QJsonDocument::fromJson(jsonData).object();

                    QJsonArray jArray = jsonObj["IP"].toArray();

                    for(int j = 0; j < jArray.size(); j++)
                    {
                        CamInfo info;
                        info.ip = jArray[j].toString();

                        int ip_p4 = info.ip.split(".").last().toInt();
    //                    qDebug() << start << " <= " << ip_p4 << " <= " << end;
    //                    qDebug() << !(start <= ip_p4 && ip_p4 <= end);

                        if(!(start <= ip_p4 && ip_p4 <= end))
                            continue;

                        if (info.ip == temp_ip)
                        {
                            info.mode = "master";
                            emit masterCam_signal(info.ip);
                        }
                        else
                            info.mode = "slave";

                        Camlist.append(info);
                    }
                }

                count++;
//                qDebug() << temp_ip << "[" << count << "]: " << result;
                if (count == countMax)
                {
                    auto a = Camlist;
                    emit scanFinish(Camlist);
                }
                mtx.unlock();
            }
        }).detach();
    }



}


void CamVC610_WebAPI_Ctrl::scanCamIP_Range(QString ipBase, int start,int end)
{
    isScan = true;
    count = 0;
    countMax = (end - start + 1);
    Camlist.clear();
    for (int i = start; i <= end; i++)
    {
        Sleep(1);
        std::thread([=]()
        {
            QString temp_ip = ipBase + QString::number(i);
            std::string res = send_url_API(temp_ip, "info", "");
            QString result = QString::fromStdString(res);
            mtx.lock();

            if(result.contains("slave not support"))
            {
    //            qDebug() << "slave";
            }
            else if(result.contains("IP") && result.contains("VC610"))
            {
    //            qDebug() << "master";
                res.erase(std::remove(res.begin(), res.end(), '\n'), res.end());
                QByteArray jsonData(res.c_str(), res.length());
                QJsonObject jsonObj = QJsonDocument::fromJson(jsonData).object();

                QJsonArray jArray = jsonObj["IP"].toArray();

                for(int j = 0; j < jArray.size(); j++)
                {
                    CamInfo info;
                    info.ip = jArray[j].toString();
                    int ip_p4 = info.ip.split(".").last().toInt();
//                    qDebug() << start << " <= " << ip_p4 << " <= " << end;
//                    qDebug() << !(start <= ip_p4 && ip_p4 <= end);

                    if(!(start <= ip_p4 && ip_p4 <= end))
                        continue;

                    if (info.ip == temp_ip)
                        info.mode = "master";
                    else
                        info.mode = "slave";

                    Camlist.append(info);
                }
            }

            count++;
//            qDebug() << temp_ip << "[" << count << "]: " << result;
            if (count == countMax)
            {
                auto a = Camlist;
                emit scanFinish(Camlist);
            }
            mtx.unlock();
        }).detach();
    }
}


int CamVC610_WebAPI_Ctrl::scanCamIP(QString ip)
{
    std::string res = send_url_API(ip, "info", "");
    QString result = QString::fromStdString(res);
    mtx.lock();

    if(result.contains("slave not support"))
    {
//            qDebug() << "slave";
        mtx.unlock();
        return 0;
    }
    else if(result.contains("IP") && result.contains("VC610"))
    {
//            qDebug() << "master";
        res.erase(std::remove(res.begin(), res.end(), '\n'), res.end());
        QByteArray jsonData(res.c_str(), res.length());
        QJsonObject jsonObj = QJsonDocument::fromJson(jsonData).object();

        QJsonArray jArray = jsonObj["IP"].toArray();
        mtx.unlock();
        return 1;
    }
    mtx.unlock();
    return -1;
}


int CamVC610_WebAPI_Ctrl::saveParam()
{
    //POST Data
    std::string json = "{\"name\":\"saveParam\"}";
    return isOperationDone(send_url_API(ip, "execute", json));
}

void CamVC610_WebAPI_Ctrl::setIP(QString ip)
{
    this->ip = ip;
}


/// private
std::string CamVC610_WebAPI_Ctrl::send_url_API(QString IP, std::string action,std::string json)
{
    std::stringstream outdata;

    CURL *curl;
    CURLcode res;

    //qDebug()<<"api:"<< QString::fromStdString(action) << ":" << QString::fromStdString(json);

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, get_url(IP, action).c_str());

    //POST Data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outdata);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);// TimeOut = 2

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
        outdata.clear();

    curl_easy_cleanup(curl);
    //qDebug()<<"res:"<<  QString::fromStdString(outdata.str());

    return outdata.str();
}


std::string CamVC610_WebAPI_Ctrl::get_url(QString IP, std::string action)
{
    std::string url_api = head.toStdString() + IP.toStdString() + api.toStdString() + action;
    //std::cout << url_api << std::endl;  // 使用 std::cout 顯示訊息並換行

    return url_api;
}


size_t CamVC610_WebAPI_Ctrl::write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::string data((const char*)ptr, (size_t)size * nmemb);

    *((std::stringstream*)stream) << data << std::endl;

    return size * nmemb;
}


int CamVC610_WebAPI_Ctrl::getStatusID(std::string resultString) {
    QJsonObject jsonObj = stdStringToQJsonObject(resultString);

    // 檢查是否是合法的 JSON
    if (!jsonObj.isEmpty()) {
        // 檢查 "id" 是否存在
        if (jsonObj.contains("id")) {
            // 取得 "id" 的值，將 QVariant 轉換為 int
            QVariant idVariant = jsonObj["id"].toVariant();
            return idVariant.toInt();
        } else {
            qDebug() << "No 'id' in JSON.";
            return -1;
        }
    } else {
        qDebug() << "Invalid JSON string.";
        return -2;
    }
}

int CamVC610_WebAPI_Ctrl::isStreaming(std::string resultString) {
    QJsonObject jsonObj = stdStringToQJsonObject(resultString);

    // 檢查是否是合法的 JSON
    if (jsonObj.contains("streamingStatus")) {
        return jsonObj["streamingStatus"].toArray()[0].toBool();
    } else {
        return -1;
    }
}

int CamVC610_WebAPI_Ctrl::isOperationDone(std::string resultString) {
    QJsonObject jsonObj = stdStringToQJsonObject(resultString);

    // 檢查 "state" 是否存在
    if (jsonObj.contains("state")) {
        // 取得 "state" 的值
        QString stateValue = jsonObj["state"].toString();

        // 檢查 "state"
        if (stateValue == "done") {
            return 1;
        } else if(stateValue == "inProgress") {
            id = getStatusID(resultString);
            return 0;
        } else if(stateValue == "error") {
            return -1;
        } else {
            qDebug() << "?state:" + stateValue;
            return -2;
        }
    } else {
        qDebug() << "No 'state' in JSON.";
        return -3;
    }
}

QJsonObject CamVC610_WebAPI_Ctrl::stdStringToQJsonObject(std::string resultString)
{
    resultString.erase(std::remove(resultString.begin(), resultString.end(), '\n'), resultString.end());
    QByteArray jsonData(resultString.c_str(), resultString.length());
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

    // 檢查是否是合法的 JSON
    if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        // 取得 JSON 對象
        return jsonDoc.object();
    } else {
        return QJsonObject();
    }
}

QString CamVC610_WebAPI_Ctrl::getCurrentDateTime()
{
    // 取得當前的日期和時間
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 將日期和時間格式化為指定的字串格式
    QString formattedDateTime = currentDateTime.toString("yyyy/MM/dd hh:mm:ss");

    return formattedDateTime;
}
