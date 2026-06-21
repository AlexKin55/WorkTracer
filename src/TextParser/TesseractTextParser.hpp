#pragma once
#include "ITextParser.hpp"

#include <cmath>

#include <tesseract/baseapi.h>

class TesseractTextParser : public ITextParser {
private:
    tesseract::TessBaseAPI* m_ocr;

public:
    TesseractTextParser();
    ~TesseractTextParser() override;

    std::string parse(const cv::Mat& image) override;
};
