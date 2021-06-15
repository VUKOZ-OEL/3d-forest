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

/**
    @file FileIndexBuilder.cpp
*/

#include <FileIndexBuilder.hpp>
#include <Vector3.hpp>
#include <cstring>
#include <iostream>

FileIndexBuilder::Settings::Settings()
{
    randomize = false;

    maxSize1 = 100000;
    // maxSize1 = 100;
    maxLevel1 = 0; // The limit is maxSize1.

    maxSize2 = 32;
    maxLevel2 = 5;
    // maxLevel2 = 2;

    bufferSize = 5 * 1024 * 1024;
}

FileIndexBuilder::Settings::~Settings()
{
}

FileIndexBuilder::FileIndexBuilder()
    : state_(STATE_NONE),
      valueTotal_(0),
      maximumTotal_(0)
{
}

FileIndexBuilder::~FileIndexBuilder()
{
}

std::string FileIndexBuilder::extension(const std::string &path)
{
    return File::replaceExtension(path, ".idx");
}

void FileIndexBuilder::index(const std::string &outputPath,
                             const std::string &inputPath,
                             const FileIndexBuilder::Settings &settings)
{
    char buffer[80];
    FileIndexBuilder builder;
    builder.start(outputPath, inputPath, settings);
    while (!builder.end())
    {
        builder.next();

        std::snprintf(buffer, sizeof(buffer), "%6.2f %%", builder.percent());
        std::cout << "\r" << buffer << std::flush;
    }
    std::cout << std::endl;
}

double FileIndexBuilder::percent() const
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

void FileIndexBuilder::start(const std::string &outputPath,
                             const std::string &inputPath,
                             const FileIndexBuilder::Settings &settings)
{
    // Initialize
    state_ = STATE_NONE;
    valueTotal_ = 0;
    maximumTotal_ = 0;

    random_ = 10;
    indexMain_.clear();
    indexNode_.clear();
    indexMainUsed_.clear();

    settings_ = settings;
    buffer_.resize(settings.bufferSize);
    bufferOut_.resize(settings.bufferSize * 2);

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

void FileIndexBuilder::openFiles()
{
    // Input
    inputLas_.open(readPath_);
    inputLas_.readHeader();

    sizePointFormat_ = inputLas_.header.pointDataRecordFormatLength();
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

    // Format
    sizePointOut_ = inputLas_.header.pointDataRecord3dForestLength();
    outputLas_.header.point_data_record_length =
        static_cast<uint16_t>(sizePointOut_);
    sizePointsOut_ = outputLas_.header.pointDataSize();
    sizeFileOut_ = sizeFile_;

    if (sizePointsOut_ > sizePoints_)
    {
        uint64_t extraBytes = sizePointsOut_ - sizePoints_;
        outputLas_.header.offset_to_wdpr += extraBytes;
        outputLas_.header.offset_to_evlr += extraBytes;
        sizeFileOut_ += extraBytes;
    }
    else if (sizePointsOut_ < sizePoints_)
    {
        uint64_t extraBytes = sizePoints_ - sizePointsOut_;
        outputLas_.header.offset_to_wdpr -= extraBytes;
        outputLas_.header.offset_to_evlr -= extraBytes;
        sizeFileOut_ -= extraBytes;
    }

    outputLas_.writeHeader();
}

void FileIndexBuilder::next()
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

        case STATE_RANDOMIZE:
            stateRandomize();
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

void FileIndexBuilder::nextState()
{
    value_ = 0;
    valueIdx_ = 0;
    maximum_ = 0;

    switch (state_)
    {
        case STATE_BEGIN:
            if ((sizePoint_ != sizePointOut_) || (settings_.randomize))
            {
                state_ = STATE_COPY_VLR;
                maximum_ = offsetPointsStart_ - offsetHeaderEnd_;
            }
            else
            {
                state_ = STATE_COPY;
                maximum_ = sizeFile_ - offsetHeaderEnd_;
            }
            break;

        case STATE_COPY_VLR:
            state_ = STATE_COPY_POINTS;
            maximum_ = sizePoints_;
            maximumIdx_ = inputLas_.header.number_of_point_records;
            break;

        case STATE_COPY_POINTS:
            state_ = STATE_COPY_EVLR;
            maximum_ = sizeFile_ - offsetPointsEnd_;
            break;

        case STATE_COPY_EVLR:
            if (settings_.randomize)
            {
                state_ = STATE_RANDOMIZE;
                maximum_ = sizePointsOut_;
                maximumIdx_ = outputLas_.header.number_of_point_records;
            }
            else
            {
                state_ = STATE_MOVE;
            }
            break;

        case STATE_RANDOMIZE:
            state_ = STATE_MOVE;
            break;

        case STATE_MOVE:
            state_ = STATE_COPY;
            maximum_ = sizeFileOut_ - offsetHeaderEnd_;
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

void FileIndexBuilder::stateCopy()
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

void FileIndexBuilder::stateCopyPoints()
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

    // Format
    if (sizePoint_ == sizePointOut_)
    {
        // Keep extra bytes garbage
        inputLas_.file().read(buffer_.data(), step);
        outputLas_.file().write(buffer_.data(), step);
    }
    else
    {
        // Extend or trim the input points
        uint8_t *in = buffer_.data();
        inputLas_.file().read(in, step);

        uint8_t *out = bufferOut_.data();
        std::memset(out, 0, sizePointOut_ * stepIdx);

        for (size_t i = 0; i < stepIdx; i++)
        {
            std::memcpy(out + (i * sizePointOut_),
                        in + (i * sizePoint_),
                        sizePointFormat_);
        }

        outputLas_.file().write(out, sizePointOut_ * stepIdx);
    }

    // Next
    value_ += step;
    valueIdx_ += stepIdx;
    valueTotal_ += step;
}

void FileIndexBuilder::stateRandomize()
{
    // Step
    uint64_t step;
    uint64_t remainIdx;
    uint64_t stepIdx;

    // stepIdx = buffer_.size() / sizePoint_;
    stepIdx = 10000; /**< @todo Rewrite function stateRandomize. */
    remainIdx = maximumIdx_ - valueIdx_;
    if (remainIdx < stepIdx)
    {
        stepIdx = remainIdx;
    }
    step = stepIdx * sizePointOut_;

    // Randomize
    uint8_t *buffer1 = buffer_.data();
    uint8_t *buffer2 = bufferOut_.data();
    uint64_t start = outputLas_.header.offset_to_point_data;
    uint64_t pos;

    for (uint64_t i = 0; i < stepIdx; i++)
    {
        pos = random_ % maximumIdx_;

        // Linear congruent pseudo-random generator
        random_ = random_ * 69069UL + 1UL;

        // Read A
        outputLas_.seek(start + (valueIdx_ + i) * sizePointOut_);
        outputLas_.file().read(buffer1, sizePointOut_);

        // Read B, Overwrite B with A
        outputLas_.seek(start + pos * sizePointOut_);
        outputLas_.file().read(buffer2, sizePointOut_);
        outputLas_.seek(start + pos * sizePointOut_);
        outputLas_.file().write(buffer1, sizePointOut_);

        // Overwrite A with B
        outputLas_.seek(start + (valueIdx_ + i) * sizePointOut_);
        outputLas_.file().write(buffer2, sizePointOut_);
    }

    // Next
    value_ += step;
    valueIdx_ += stepIdx;
    valueTotal_ += step;
}

void FileIndexBuilder::stateMove()
{
    // Move
    inputLas_.close();
    outputLas_.close();

    // Reopen
    readPath_ = writePath_;
    writePath_ = File::tmpname(outputPath_);
    openFiles();
}

void FileIndexBuilder::stateMainBegin()
{
    // Insert begin
#if 1
    // Cube
    Vector3<double> dim(inputLas_.header.max_x - inputLas_.header.min_x,
                        inputLas_.header.max_y - inputLas_.header.min_y,
                        inputLas_.header.max_z - inputLas_.header.min_z);

    double dimMax = dim.max();

    Aabb<double> box;
    box.set(inputLas_.header.min_x,
            inputLas_.header.min_y,
            inputLas_.header.min_z,
            inputLas_.header.min_x + dimMax,
            inputLas_.header.min_y + dimMax,
            inputLas_.header.min_z + dimMax);
#else
    // Cuboid
    Aabb<double> box;
    box.set(inputLas_.header.min_x,
            inputLas_.header.min_y,
            inputLas_.header.min_z,
            inputLas_.header.max_x,
            inputLas_.header.max_y,
            inputLas_.header.max_z);
#endif
    indexMain_.insertBegin(box, settings_.maxSize1, settings_.maxLevel1);

    // Initial file offset
    inputLas_.seekPointData();
}

void FileIndexBuilder::stateMainInsert()
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
    inputLas_.file().read(buffer, step);

    double x;
    double y;
    double z;

    for (uint64_t i = 0; i < stepIdx; i++)
    {
        inputLas_.transform(x, y, z, buffer + (i * sizePoint_));
        (void)indexMain_.insert(x, y, z);
    }

    // Next
    value_ += step;
    valueIdx_ += stepIdx;
    valueTotal_ += step;
}

void FileIndexBuilder::stateMainEnd()
{
    indexMain_.insertEnd();

    // Write main index
    std::string indexPath = extension(outputPath_);
    indexFile_.open(indexPath, "w");
    indexMain_.write(indexFile_);

    // Next initial file offset
    inputLas_.seekPointData();
}

void FileIndexBuilder::stateMainSort()
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
    const FileIndex::Node *node;

    for (uint64_t i = 0; i < stepIdx; i++)
    {
        point = buffer + (i * sizePoint_);
        inputLas_.transform(x, y, z, point);

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

void FileIndexBuilder::stateNodeBegin()
{
}

static int DatabaseBuilderCmp(const void *a, const void *b)
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

void FileIndexBuilder::stateNodeInsert()
{
    // Step
    uint64_t step;
    FileIndex::Node *node;

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
    double x;
    double y;
    double z;

    outputLas_.seek(start + (node->from * sizePoint_));
    outputLas_.file().read(buffer, step);

    std::vector<double> coords;
    coords.resize(node->size * 3);
    for (uint64_t i = 0; i < node->size; i++)
    {
        point = buffer + (i * sizePoint_);
        inputLas_.transform(x, y, z, point);
        coords[i * 3 + 0] = x;
        coords[i * 3 + 1] = y;
        coords[i * 3 + 2] = z;
    }

    Aabb<double> box;
    box.set(coords);
    // box = indexMain_.boundary(node, indexMain_.boundary());

    indexNode_.clear();
    indexNode_.insertBegin(box, settings_.maxSize2, settings_.maxLevel2, true);

    for (uint64_t i = 0; i < node->size; i++)
    {
        // point = buffer + (i * sizePoint_);
        // inputLas_.transform(x, y, z, point);
        x = coords[i * 3 + 0];
        y = coords[i * 3 + 1];
        z = coords[i * 3 + 2];
        bufferCodes[i * 2 + 0] = indexNode_.insert(x, y, z);
        bufferCodes[i * 2 + 1] = i;
    }

    indexNode_.insertEnd();
    node->offset = indexFile_.offset();
    indexNode_.write(indexFile_);

    // Sort
    size_t size = sizeof(uint64_t) * 2;
    size_t n = bufferCodes.size() / 2;
    std::qsort(bufferCodes.data(), n, size, DatabaseBuilderCmp);

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

void FileIndexBuilder::stateNodeEnd()
{
    indexFile_.seek(0);
    indexMain_.write(indexFile_);

    indexFile_.close();
}

void FileIndexBuilder::stateEnd()
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
