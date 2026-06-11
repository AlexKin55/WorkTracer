#include "TesseractTextParser.hpp"
#include <vector>
#include <algorithm>

struct OcrWord {
    std::string text;
    int x1;
    int y1;
    int x2;
    int y2;
};

TesseractTextParser::TesseractTextParser() {
    ocr = new tesseract::TessBaseAPI();
    ocr->Init(nullptr, "eng+rus", tesseract::OEM_DEFAULT);
    ocr->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
}

TesseractTextParser::~TesseractTextParser() {
    ocr->End();
    delete ocr;
}

std::string TesseractTextParser::parse(const cv::Mat& image) {
    if (image.empty()) {
        return {};
    }

    ocr->SetImage(image.data, image.cols, image.rows, image.channels(), image.step1());
    ocr->Recognize(0);

    std::vector<OcrWord> words;
    tesseract::ResultIterator* ri = ocr->GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

    if (ri) {
        do {
            const char* word_ptr = ri->GetUTF8Text(level);
            if (word_ptr) {
                std::string word_str(word_ptr);
                if (!word_str.empty()) {
                    OcrWord w;
                    w.text = word_str;
                    ri->BoundingBox(level, &w.x1, &w.y1, &w.x2, &w.y2);
                    words.push_back(w);
                }
                delete[] word_ptr;
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
        int current_y = words[0].y1;
        int last_x2 = words[0].x2;
        
        for (const auto& w : words) {
            if (std::abs(w.y1 - current_y) >= 8) {
                result += "\n";
                current_y = w.y1;
                last_x2 = w.x1;
            }
            int gap = w.x1 - last_x2;
            if (gap > 90) {
                result += "   |   ";
            } else if (last_x2 != w.x1) {
                result += " ";
            }
            result += w.text;
            last_x2 = w.x2;
        }
    }
    return result;
}
