/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file IndexFileBuilder.cpp */

#include <cstring>

#include <Endian.hpp>
#include <IndexFileBuilder.hpp>
#include <Log.hpp>
#include <Vector3.hpp>

// Define to keep the same order of LAS points.
//#define INDEX_FILE_BUILDER_DEBUG_SAME_ORDER

#define indexFileBuilderCoordinate(ptr)                                        \
    static_cast<double>(static_cast<int32_t>(ltoh32(ptr)))

IndexFileBuilder::Settings::Settings()
{
    verbose = false;

    maxSize1 = 100000;
    maxLevel1 = 0; // The limit is maxSize1.

    maxSize2 = 32;
    maxLevel2 = 5;
    // maxLevel2 = 2;

    bufferSize = 5 * 1024 * 1024;
}

IndexFileBuilder::Settings::~Settings()
{
}

IndexFileBuilder::IndexFileBuilder()
    : state_(STATE_NONE),
      valueTotal_(0),
      maximumTotal_(0)
{
}

IndexFileBuilder::~IndexFileBuilder()
{
}

std::string IndexFileBuilder::extension(const std::string &path)
{
    return File::replaceExtension(path, ".idx");
}

void IndexFileBuilder::index(const std::string &outputPath,
                             const std::string &inputPath,
                             const IndexFileBuilder::Settings &settings)
{
    char buffer[80];
    IndexFileBuilder builder;

    builder.start(outputPath, inputPath, settings);

    while (!builder.end())
    {
        builder.next();

        if (settings.verbose)
        {
            std::snprintf(buffer, sizeof(buffer), "%6.2f%%", builder.percent());
            std::cout << "\r" << buffer << std::flush;
        }
    }

    if (settings.verbose)
    {
        std::cout << std::endl;
    }
}

double IndexFileBuilder::percent() const
{
    if (maximumTotal_ == 0)
    {
        return 100.0;
    }
    else
    {
        return 100.0 * (static_cast<double>(valueTotal_) /
                        static_cast<double>(maximumTotal_));
    }
}

void IndexFileBuilder::start(const std::string &outputPath,
                             const std::string &inputPath,
                             const IndexFileBuilder::Settings &settings)
{
    // Initialize
    state_ = STATE_NONE;
    valueTotal_ = 0;
    maximumTotal_ = 0;

    boundary_.clear();
    rgbMax_ = 0;
    intensityMax_ = 0;

    indexMain_.clear();
    indexNode_.clear();
    indexMainUsed_.clear();

    settings_ = settings;
    buffer_.resize(settings.bufferSize);
    bufferOut_.resize(settings.bufferSize);

    // Open files
    inputPath_ = inputPath;
    outputPath_ = outputPath;
    readPath_ = inputPath_;
    writePath_ = File::tmpname(outputPath_);

    openFiles();

    // Maximum total progress
    state_ = STATE_BEGIN;
    while (!end())
    {
        nextState();
        maximumTotal_ += maximum_;
    }

    // Initial state
    state_ = STATE_BEGIN;
    nextState();
}

void IndexFileBuilder::openFiles()
{
    // Input
    inputLas_.open(readPath_);
    inputLas_.readHeader();

    sizePointFormat_ = inputLas_.header.pointDataRecordLengthFormat();
    sizePoint_ = inputLas_.header.point_data_record_length;
    sizePoints_ = inputLas_.header.pointDataSize();
    sizeFile_ = inputLas_.file().size();

    offsetHeaderEnd_ = inputLas_.file().offset();
    offsetPointsStart_ = inputLas_.header.offset_to_point_data;
    offsetPointsEnd_ = offsetPointsStart_ + sizePoints_;

    // Output
    outputLas_.create(writePath_);
    outputLas_.header = inputLas_.header;
    outputLas_.header.setGeneratingSoftware();

    // Convert to LAS 1.4+
    if ((outputLas_.header.version_major == 1) &&
        (outputLas_.header.version_minor < 4))
    {
        outputLas_.header.version_minor = 4;

        switch (outputLas_.header.point_data_record_format)
        {
            case 0:
            case 1:
                outputLas_.header.point_data_record_format = 6;
                break;
            case 2:
            case 3:
                outputLas_.header.point_data_record_format = 7;
                break;
            case 4:
                outputLas_.header.point_data_record_format = 9;
                break;
            case 5:
                outputLas_.header.point_data_record_format = 10;
                break;

            default:
                // Do nothing
                break;
        }
    }

    uint64_t headerSize = inputLas_.header.header_size;
    uint64_t headerExtra = headerSize - offsetHeaderEnd_;
    uint64_t offsetHeaderEndVersionOut = outputLas_.header.versionHeaderSize();
    uint64_t headerSizeOut = offsetHeaderEndVersionOut + headerExtra;

    outputLas_.header.header_size = static_cast<uint16_t>(headerSizeOut);
    offsetHeaderEndOut_ = offsetHeaderEndVersionOut;

    uint64_t offsetPointsStartDiff = headerSizeOut - headerSize;
    offsetPointsStartOut_ = offsetPointsStart_ + offsetPointsStartDiff;
    outputLas_.header.addOffsetPointData(offsetPointsStartDiff);
    outputLas_.header.addOffsetWdpr(offsetPointsStartDiff);
    outputLas_.header.addOffsetEvlr(offsetPointsStartDiff);

    // Format
    sizePointOut_ = outputLas_.header.pointDataRecordLength3dForest();
    outputLas_.header.point_data_record_length =
        static_cast<uint16_t>(sizePointOut_);
    sizePointsOut_ = outputLas_.header.pointDataSize();
    offsetPointsEndOut_ = offsetPointsStartOut_ + sizePointsOut_;
    sizeFileOut_ = sizeFile_;
    sizeFileOut_ += offsetPointsStartDiff;

    if (sizePointsOut_ > sizePoints_)
    {
        uint64_t extraBytes = sizePointsOut_ - sizePoints_;
        outputLas_.header.addOffsetWdpr(extraBytes);
        outputLas_.header.addOffsetEvlr(extraBytes);
        sizeFileOut_ += extraBytes;
    }
    else if (sizePointsOut_ < sizePoints_)
    {
        uint64_t extraBytes = sizePoints_ - sizePointsOut_;
        outputLas_.header.subOffsetWdpr(extraBytes);
        outputLas_.header.subOffsetEvlr(extraBytes);
        sizeFileOut_ -= extraBytes;
    }

    outputLas_.writeHeader();
}

void IndexFileBuilder::next()
{
    // Continue
    switch (state_)
    {
        case STATE_COPY_VLR:
            stateCopy();
            break;

        case STATE_COPY_POINTS:
            stateCopyPoints();
            break;

        case STATE_COPY_EVLR:
            stateCopy();
            break;

        case STATE_MOVE:
            stateMove();
            break;

        case STATE_COPY:
            stateCopy();
            break;

        case STATE_MAIN_BEGIN:
            stateMainBegin();
            break;

        case STATE_MAIN_INSERT:
            stateMainInsert();
            break;

        case STATE_MAIN_END:
            stateMainEnd();
            break;

        case STATE_MAIN_SORT:
            stateMainSort();
            break;

        case STATE_NODE_BEGIN:
            stateNodeBegin();
            break;

        case STATE_NODE_INSERT:
            stateNodeInsert();
            break;

        case STATE_NODE_END:
            stateNodeEnd();
            break;

        case STATE_END:
            stateEnd();
            break;

        default:
        case STATE_NONE:
        case STATE_BEGIN:
            break;
    }

    // Next
    if (value_ == maximum_)
    {
        nextState();
    }
}

void IndexFileBuilder::nextState()
{
    value_ = 0;
    valueIdx_ = 0;
    maximum_ = 0;

    switch (state_)
    {
        case STATE_BEGIN:
            state_ = STATE_COPY_VLR;
            maximum_ = offsetPointsStart_ - offsetHeaderEnd_;
            break;

        case STATE_COPY_VLR:
            state_ = STATE_COPY_POINTS;
            maximum_ = sizePoints_;
            maximumIdx_ = inputLas_.header.number_of_point_records;
            start_ = 0;
            current_ = 0;
            max_ = maximumIdx_;
#ifndef INDEX_FILE_BUILDER_DEBUG_SAME_ORDER
            step_ = max_ / settings_.maxSize1;
            if (max_ % settings_.maxSize1 > 0)
            {
                step_++;
            }
#else
            step_ = 1;
#endif /* INDEX_FILE_BUILDER_DEBUG_SAME_ORDER */
            break;

        case STATE_COPY_POINTS:
            state_ = STATE_COPY_EVLR;
            maximum_ = sizeFile_ - offsetPointsEnd_;
            break;

        case STATE_COPY_EVLR:
            state_ = STATE_MOVE;
            break;

        case STATE_MOVE:
            state_ = STATE_COPY;
            maximum_ = sizeFileOut_ - offsetHeaderEndOut_;
            break;

        case STATE_COPY:
            state_ = STATE_MAIN_BEGIN;
            break;

        case STATE_MAIN_BEGIN:
            state_ = STATE_MAIN_INSERT;
            maximum_ = sizePointsOut_;
            maximumIdx_ = outputLas_.header.number_of_point_records;
            break;

        case STATE_MAIN_INSERT:
            state_ = STATE_MAIN_END;
            break;

        case STATE_MAIN_END:
            state_ = STATE_MAIN_SORT;
            maximum_ = sizePointsOut_;
            maximumIdx_ = outputLas_.header.number_of_point_records;
            break;

        case STATE_MAIN_SORT:
            state_ = STATE_NODE_BEGIN;
            break;

        case STATE_NODE_BEGIN:
            state_ = STATE_NODE_INSERT;
            maximum_ = sizePointsOut_;
            maximumIdx_ = indexMain_.size();
            break;

        case STATE_NODE_INSERT:
            state_ = STATE_NODE_END;
            break;

        case STATE_NODE_END:
            state_ = STATE_END;
            break;

        case STATE_END:
            state_ = STATE_NONE;
            break;

        default:
        case STATE_NONE:
            break;
    }
}

void IndexFileBuilder::stateCopy()
{
    // Step
    uint64_t step;
    uint64_t remain;

    step = buffer_.size();
    remain = maximum_ - value_;
    if (remain < step)
    {
        step = remain;
    }

    // Copy
    inputLas_.file().read(buffer_.data(), step);
    outputLas_.file().write(buffer_.data(), step);

    // Next
    value_ += step;
    valueTotal_ += step;
}

void IndexFileBuilder::formatPoint(uint8_t *pout, const uint8_t *pin) const
{
    // i: edge:1, scan:1, number_of_returns:3, return_number:3
    // o:                 number_of_returns:4, return_number:4
    // i:             classification_flags:3, classification:5
    // o: edge:1, scan:1,    scanner:2, classification_flags:4
    uint32_t pi14 = pin[14];
    uint32_t pi15 = pin[15];
    uint32_t po = (pi14 & 0x07U) | ((pi14 & 0x38U) << 1);
    pout[14] = static_cast<uint8_t>(po);

    po = (pi14 & 0xc0U) | (pi15 >> 5);
    pout[15] = static_cast<uint8_t>(po);
    pout[16] = static_cast<uint8_t>(pi15 & 0x1fU);

    // Angle by 0.006 degree from [-90,90] to [-15000, 15000]
    int8_t angle = static_cast<int8_t>(pin[16]);
    double angled = 166.666667 * static_cast<double>(angle);
    int16_t angle16 = static_cast<int16_t>(angled);
    htol16(&pout[18], static_cast<uint16_t>(angle16));

    // source_id
    pout[20] = pin[18];
    pout[21] = pin[19];

    // GPS time
    if ((inputLas_.header.point_data_record_format == 1) ||
        (inputLas_.header.point_data_record_format > 2))
    {
        copy64(&pout[22], &pin[20]);
    }
    else
    {
        std::memset(&pout[22], 0, 8);
    }

    // RGB
    if (inputLas_.header.point_data_record_format == 2)
    {
        std::memcpy(&pout[30], &pin[20], 6);
    }
    else if ((inputLas_.header.point_data_record_format == 3) ||
             (inputLas_.header.point_data_record_format == 5))
    {
        std::memcpy(&pout[30], &pin[28], 6);
    }

    // NIR
    if ((outputLas_.header.point_data_record_format == 8) ||
        (outputLas_.header.point_data_record_format == 10))
    {
        pout[36] = 0;
        pout[37] = 0;
    }

    // Wave
    if (inputLas_.header.point_data_record_format == 4)
    {
        if (outputLas_.header.point_data_record_format == 9)
        {
            std::memcpy(&pout[30], &pin[28], 29);
        }
        else
        {
            std::memcpy(&pout[38], &pin[28], 29);
        }
    }
    else if (inputLas_.header.point_data_record_format == 5)
    {
        if (outputLas_.header.point_data_record_format == 9)
        {
            std::memcpy(&pout[30], &pin[34], 29);
        }
        else
        {
            std::memcpy(&pout[38], &pin[34], 29);
        }
    }
}

void IndexFileBuilder::stateCopyPoints()
{
    // Step
    uint64_t step;
    uint64_t remainIdx;
    size_t stepIdx;

    stepIdx = buffer_.size() / sizePoint_;
    remainIdx = maximumIdx_ - valueIdx_;
    if (remainIdx < static_cast<uint64_t>(stepIdx))
    {
        stepIdx = static_cast<size_t>(remainIdx);
    }
    step = stepIdx * sizePoint_;

    // Buffers
    if (bufferOut_.size() < sizePointOut_ * stepIdx)
    {
        bufferOut_.resize(sizePointOut_ * stepIdx);
    }

    uint64_t start = inputLas_.header.offset_to_point_data;
    uint8_t *in = buffer_.data();
    uint8_t *bufferOut = bufferOut_.data();
    uint8_t *out;

    // Clear the output buffer
    std::memset(bufferOut, 0, sizePointOut_ * stepIdx);

    // Coordinates without scaling
    coords_.resize(stepIdx * 3);

    // Point formatting
    bool hasDifferentFormat;
    if ((inputLas_.header.point_data_record_format < 6) &&
        (outputLas_.header.point_data_record_format >= 6))
    {
        hasDifferentFormat = true;
    }
    else
    {
        hasDifferentFormat = false;
    }

    // To find maximums to normalize these values
    uint32_t intensity;
    uint32_t rgb;

    bool hasColor;
    if ((outputLas_.header.point_data_record_format == 7) ||
        (outputLas_.header.point_data_record_format == 8) ||
        (outputLas_.header.point_data_record_format == 10))
    {
        hasColor = true;
    }
    else
    {
        hasColor = false;
    }

    // Process one step of the input
    for (size_t i = 0; i < stepIdx; i++)
    {
        // Reorder
        inputLas_.seek(start + (current_ * sizePoint_));
        current_ += step_;
        if (current_ >= max_)
        {
            start_++;
            current_ = start_;
        }

        // Read input
        inputLas_.file().read(in, sizePoint_);
        out = bufferOut + (i * sizePointOut_);

        // Copy
        std::memcpy(out, in, sizePoint_); // sizePointFormat_

        // Boundary of points without scaling and offset
        coords_[i * 3 + 0] = indexFileBuilderCoordinate(out + 0);
        coords_[i * 3 + 1] = indexFileBuilderCoordinate(out + 4);
        coords_[i * 3 + 2] = indexFileBuilderCoordinate(out + 8);

        // Format
        if (hasDifferentFormat)
        {
            formatPoint(out, in);
        }

        // Find maximums to normalize these values later
        intensity = ltoh16(out + 12);
        if (intensity > intensityMax_)
        {
            intensityMax_ = intensity;
        }

        if (hasColor)
        {
            rgb = ltoh16(out + 30);
            rgb += ltoh16(out + 32);
            rgb += ltoh16(out + 34);

            if (rgb > rgbMax_)
            {
                rgbMax_ = rgb;
            }
        }
    }

    // Write this step to the output
    outputLas_.file().write(bufferOut, sizePointOut_ * stepIdx);

    // Boundary without scaling
    Box<double> box;
    box.set(coords_);
    boundary_.extend(box);

    // Next
    value_ += step;
    valueIdx_ += stepIdx;
    valueTotal_ += step;
}

void IndexFileBuilder::stateMove()
{
    // Move
    inputLas_.close();
    outputLas_.close();

    // Reopen
    readPath_ = writePath_;
    writePath_ = File::tmpname(outputPath_);
    openFiles();
}

void IndexFileBuilder::stateMainBegin()
{
    // Cuboid to Cube boundary for index L1
    Vector3<double> dim(boundary_.max(0) - boundary_.min(0),
                        boundary_.max(1) - boundary_.min(1),
                        boundary_.max(2) - boundary_.min(2));

    double dimMax = dim.max();

    Box<double> box;

    box.set(boundary_.min(0),
            boundary_.min(1),
            boundary_.min(2),
            boundary_.min(0) + dimMax,
            boundary_.min(1) + dimMax,
            boundary_.min(2) + dimMax);

    // Insert begin
    indexMain_.insertBegin(box,
                           boundary_,
                           settings_.maxSize1,
                           settings_.maxLevel1);

    // Initial file offset
    inputLas_.seekPointData();
}

void IndexFileBuilder::stateMainInsert()
{
    // Step
    uint64_t step;
    uint64_t remainIdx;
    uint64_t stepIdx;

    stepIdx = buffer_.size() / sizePoint_;
    remainIdx = maximumIdx_ - valueIdx_;
    if (remainIdx < stepIdx)
    {
        stepIdx = remainIdx;
    }
    step = stepIdx * sizePoint_;

    // Points
    uint8_t *buffer = buffer_.data();
    uint8_t *point;
    inputLas_.file().read(buffer, step);

    double x;
    double y;
    double z;

    for (uint64_t i = 0; i < stepIdx; i++)
    {
        point = buffer + (i * sizePoint_);
        x = indexFileBuilderCoordinate(point + 0);
        y = indexFileBuilderCoordinate(point + 4);
        z = indexFileBuilderCoordinate(point + 8);
        (void)indexMain_.insert(x, y, z);
    }

    // Next
    value_ += step;
    valueIdx_ += stepIdx;
    valueTotal_ += step;
}

void IndexFileBuilder::stateMainEnd()
{
    indexMain_.insertEnd();

    // Write main index
    std::string indexPath = extension(outputPath_);
    indexFile_.open(indexPath, "w");
    indexMain_.write(indexFile_);

    // Next initial file offset
    inputLas_.seekPointData();
}

void IndexFileBuilder::stateMainSort()
{
    // Step
    uint64_t step;
    uint64_t remainIdx;
    uint64_t stepIdx;

    stepIdx = buffer_.size() / sizePoint_;
    remainIdx = maximumIdx_ - valueIdx_;
    if (remainIdx < stepIdx)
    {
        stepIdx = remainIdx;
    }
    step = stepIdx * sizePoint_;

    // Points
    uint8_t *buffer = buffer_.data();
    uint8_t *point;
    uint64_t start = outputLas_.header.offset_to_point_data;
    uint64_t pos;

    inputLas_.file().read(buffer, step);

    double x;
    double y;
    double z;
    uint16_t intensity;
    uint16_t color;
    const IndexFile::Node *node;

    for (uint64_t i = 0; i < stepIdx; i++)
    {
        point = buffer + (i * sizePoint_);
        x = indexFileBuilderCoordinate(point + 0);
        y = indexFileBuilderCoordinate(point + 4);
        z = indexFileBuilderCoordinate(point + 8);

        // Normalize unscaled values
        if (intensityMax_ > 0 && intensityMax_ < 256)
        {
            intensity = ltoh16(point + 12);
            intensity = static_cast<uint16_t>(
                (static_cast<float>(intensity) / 255.0F) * 65535.0F);
            htol16(point + 12, intensity);
        }

        if (rgbMax_ > 0 && rgbMax_ < 766)
        {
            color = ltoh16(point + 30);
            color = static_cast<uint16_t>((static_cast<float>(color) / 255.0F) *
                                          65535.0F);
            htol16(point + 30, color);

            color = ltoh16(point + 32);
            color = static_cast<uint16_t>((static_cast<float>(color) / 255.0F) *
                                          65535.0F);
            htol16(point + 32, color);

            color = ltoh16(point + 34);
            color = static_cast<uint16_t>((static_cast<float>(color) / 255.0F) *
                                          65535.0F);
            htol16(point + 34, color);
        }

        // Update node
        node = indexMain_.selectNode(indexMainUsed_, x, y, z);
        if (node)
        {
            pos = indexMainUsed_[node]++;
            pos += node->from;
            outputLas_.seek(start + (pos * sizePoint_));
            outputLas_.file().write(point, sizePoint_);
        }
    }

    // Next
    value_ += step;
    valueIdx_ += stepIdx;
    valueTotal_ += step;
}

void IndexFileBuilder::stateNodeBegin()
{
}

static int FileIndexBuilderCmp(const void *a, const void *b)
{
    uint64_t c1 = *static_cast<const uint64_t *>(a);
    uint64_t c2 = *static_cast<const uint64_t *>(b);

    if (c1 < c2)
    {
        return -1;
    }

    if (c1 > c2)
    {
        return 1;
    }

    return 0;
}

void IndexFileBuilder::stateNodeInsert()
{
    // Step
    uint64_t step;
    IndexFile::Node *node;

    node = indexMain_.at(static_cast<size_t>(valueIdx_));
    step = node->size * sizePoint_;

    // Index
    std::vector<uint8_t> bufferNode;
    bufferNode.resize(step);
    std::vector<uint64_t> bufferCodes;
    bufferCodes.resize(node->size * 2); // pair { code, index }

    uint64_t start = outputLas_.header.offset_to_point_data;
    uint8_t *buffer = bufferNode.data();
    uint8_t *point;

    outputLas_.seek(start + (node->from * sizePoint_));
    outputLas_.file().read(buffer, step);

    // Actual boundary of this page
    coords_.resize(node->size * 3);
    for (uint64_t i = 0; i < node->size; i++)
    {
        point = buffer + (i * sizePoint_);
        coords_[i * 3 + 0] = indexFileBuilderCoordinate(point + 0);
        coords_[i * 3 + 1] = indexFileBuilderCoordinate(point + 4);
        coords_[i * 3 + 2] = indexFileBuilderCoordinate(point + 8);
    }

    Box<double> box;
    box.set(coords_);

    // Start new node
    indexNode_.clear();
    indexNode_.insertBegin(box,
                           box,
                           settings_.maxSize2,
                           settings_.maxLevel2,
                           true);

    for (uint64_t i = 0; i < node->size; i++)
    {
        bufferCodes[i * 2 + 0] = indexNode_.insert(coords_[i * 3 + 0],
                                                   coords_[i * 3 + 1],
                                                   coords_[i * 3 + 2]);
        bufferCodes[i * 2 + 1] = i;
    }

    indexNode_.insertEnd();
    node->offset = indexFile_.offset();
    indexNode_.write(indexFile_);

    // Sort
    size_t size = sizeof(uint64_t) * 2;
    size_t n = bufferCodes.size() / 2;
#ifndef INDEX_FILE_BUILDER_DEBUG_SAME_ORDER
    std::qsort(bufferCodes.data(), n, size, FileIndexBuilderCmp);
#endif /* INDEX_FILE_BUILDER_DEBUG_SAME_ORDER */

    std::vector<uint8_t> bufferNodeOut;
    bufferNodeOut.resize(step);
    uint8_t *bufferOut = bufferNodeOut.data();
    uint8_t *pointOut;

    for (uint64_t i = 0; i < node->size; i++)
    {
        point = buffer + (bufferCodes[i * 2 + 1] * sizePoint_);
        pointOut = bufferOut + (i * sizePoint_);
        std::memcpy(pointOut, point, sizePoint_);
    }

    // Write sorted points
    outputLas_.seek(start + (node->from * sizePoint_));
    outputLas_.file().write(bufferOut, step);

    // Next
    value_ += step;
    valueIdx_ += 1;
    valueTotal_ += step;
}

void IndexFileBuilder::stateNodeEnd()
{
    indexFile_.seek(0);
    indexMain_.write(indexFile_);

    indexFile_.close();
}

void IndexFileBuilder::stateEnd()
{
    // Cleanup and create the final output file
    inputLas_.close();
    outputLas_.close();

    if (readPath_ != inputPath_)
    {
        File::remove(readPath_);
    }

    File::move(outputPath_, writePath_);
}
