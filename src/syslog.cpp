#include "syslog.h"

bool ICACHE_FLASH_ATTR Syslog::begin(AsyncWebSocket* socket, bool dbg, std::deque<String>* syslogDeque) {
    this->ws = socket;
    this->debug = dbg;
    this->syslogDeque = syslogDeque;
    return true;
}

bool ICACHE_FLASH_ATTR Syslog::logln(String text) {
    if (print(text, true, false)) return true;
    return false;
}

bool ICACHE_FLASH_ATTR Syslog::logln(int text, bool hex) {
    if (hex) {
        if (print(String(text), true, true)) return true;
    }
    else {
        if (print(String(text), true, false)) return true;
    }
    return false;
}

bool ICACHE_FLASH_ATTR Syslog::log(String text) {
    if (print(text, false, false)) return true;
    return false;
}

bool ICACHE_FLASH_ATTR Syslog::log(int text, bool hex) {
    if (hex) {
        if (print(String(text), false, true)) return true;
    }
    else {
        if (print(String(text), false, false)) return true;
    }
    return false;
}

bool ICACHE_FLASH_ATTR Syslog::print(String text, bool ln, bool hex = false) {
    if (ln) {
        if (hex) {
            char h[2];
            sprintf(h, "%X", atoi(text.c_str()));
            String t = &h[0];
            t = t + "\n";
            Serial.println(t);
            this->send(t);
        }
        else {
            String t = text + "\n";
            Serial.println(text);
            this->send(t);
        }
    }
    else {
        if (hex) {
            char h[2];
            sprintf(h, "%X", atoi(text.c_str()));
            String t = &h[0];
            Serial.print(t);
            this->send(t);
        }
        else {
            Serial.print(text);
            this->send(text);
        }
    }
    return true;
}

bool ICACHE_FLASH_ATTR Syslog::send(String t) {
    if (this->debug) {
        StaticJsonDocument<256> jsonDocForWs;
        jsonDocForWs["command"] = "syslog";
        jsonDocForWs["text"] = t;
        size_t len = measureJson(jsonDocForWs);
        AsyncWebSocketMessageBuffer * buffer = this->ws->makeBuffer(len);
        if (buffer) {
            serializeJson(jsonDocForWs, (char *)buffer->get(), len + 1);
            this->ws->textAll(buffer);
        }
        
        //Serial.print("Entries in Deque: ");
        //Serial.println(this->syslogDeque->size());
        if (this->syslogDeque->size() >= 200) {
            this->syslogDeque->pop_front();
            //Serial.println("pop_front");
        }

        if (t.length() < 20) { //replace it with 'var + t' if the string is shorter than the timestamp
            String var = this->syslogDeque->at(this->syslogDeque->size()-1);  //get the last element
            //Serial.println("Replace last entry");
            var += t;
            this->syslogDeque->pop_back();  //Pop last element
            this->syslogDeque->push_back(var); //Push updated last element
        }
        else {
            this->syslogDeque->push_back(t);
        }

        //this->StringArray[this->StringArray->length()+1] = t;

        /*
        DynamicJsonDocument _doc(24576);

        JsonArray _data = _doc.createNestedArray("syslog_export");
        
        if (this->data.size() > 200) {
            this->data.remove(0);
            Serial.println("Deleted entry 0 in Syslog-JsonArray");
        }

        for (size_t i = 0; i < this->data.size(); i++) {
            _data.add(this->data[i]);
        }

        Serial.print("Entries in JsonArray: ");
        Serial.println(this->data.size());

        if (t.length() < 20) { //replace it with 'var + t' if the string is shorter than the timestamp
            size_t size = _data.size();
            String var = _data.getElement(size - 1);  //get the last element
            //Serial.println("Replace last entry");
            var += t;
            this->data.remove(size-1);
            this->data.add(var);
            //this->jsonDoc->garbageCollect();
        }
        else {
            this->data.add(t);
        }
        this->jsonDoc->clear();
        this->jsonDoc = &_doc; */
    }
    return true;
}
