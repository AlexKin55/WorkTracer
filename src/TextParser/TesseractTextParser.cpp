#include "TesseractTextParser.hpp"

#include <algorithm>
#include <vector>

struct OcrWord {
    std::string text;
    int x1;
    int y1;
    int x2;
    int y2;
};

TesseractTextParser::TesseractTextParser() {
    m_ocr = new tesseract::TessBaseAPI();
    m_ocr->Init(nullptr, "eng+rus", tesseract::OEM_DEFAULT);
    m_ocr->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
}

TesseractTextParser::~TesseractTextParser() {
    m_ocr->End();
    delete m_ocr;
}

std::string TesseractTextParser::parse(const cv::Mat& image) {
    if (image.empty()) {
        return {};
    }

    m_ocr->SetImage(image.data, image.cols, image.rows, image.channels(), image.step1());
    m_ocr->Recognize(0);

    std::vector<OcrWord> words;
    tesseract::ResultIterator* ri = m_ocr->GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

    if (ri) {
        do {
            const char* ptr = ri->GetUTF8Text(level);
            if (ptr) {
                std::string word(ptr);
                if (!word.empty()) {
                    OcrWord w;
                    w.text = word;
                    ri->BoundingBox(level, &w.x1, &w.y1, &w.x2, &w.y2);
                    words.push_back(w);
                }
                delete[] ptr;
            }
        } while (ri->Next(level));
    }

    std::sort(words.begin(), words.end(), [](const OcrWord& a, const OcrWord& b) {
        if (std::abs(a.y1 - b.y1) < 8) {
            return a.x1 < b.x1;
        }
        return a.y1 < b.y1;
    });

    std::string result = "";
    if (!words.empty()) {
        int cY = words[0].y1;
        int lX2 = words[0].x2;
        
        for (const auto& w : words) {
            if (std::abs(w.y1 - cY) >= 8) {
                result += "\n";
                cY = w.y1;
                lX2 = w.x1;
            }
            int gap = w.x1 - lX2;
            if (gap > 90) {
                result += "   |   ";
            } else if (lX2 != w.x1) {
                result += " ";
            }
            result += w.text;
            lX2 = w.x2;
        }
    }
    return result;
}
