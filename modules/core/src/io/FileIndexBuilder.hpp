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

/** @file FileIndexBuilder.hpp */

#ifndef FILE_INDEX_BUILDER_HPP
#define FILE_INDEX_BUILDER_HPP

#include <FileChunk.hpp>
#include <FileIndex.hpp>
#include <FileLas.hpp>
#include <map>
#include <string>
#include <vector>

/** File Index Index Builder. */
class FileIndexBuilder
{
public:
    /** File Index Builder Settings. */
    class Settings
    {
    public:
        bool verbose;

        size_t maxSize1;
        size_t maxSize2;

        size_t maxLevel1;
        size_t maxLevel2;

        size_t bufferSize;

        Settings();
        ~Settings();
    };

    FileIndexBuilder();
    ~FileIndexBuilder();

    void start(const std::string &outputPath,
               const std::string &inputPath,
               const FileIndexBuilder::Settings &settings);

    void next();

    bool end() const { return state_ == STATE_NONE; }

    double percent() const;

    static std::string extension(const std::string &path);

    static void index(const std::string &outputPath,
                      const std::string &inputPath,
                      const FileIndexBuilder::Settings &settings);

protected:
    // State
    /** File Index Builder State. */
    enum State
    {
        STATE_NONE,
        STATE_BEGIN,
        STATE_COPY_VLR,
        STATE_COPY_POINTS,
        STATE_COPY_EVLR,
        STATE_MOVE,
        STATE_COPY,
        STATE_MAIN_BEGIN,
        STATE_MAIN_INSERT,
        STATE_MAIN_END,
        STATE_MAIN_SORT,
        STATE_NODE_BEGIN,
        STATE_NODE_INSERT,
        STATE_NODE_END,
        STATE_END
    };

    State state_;

    uint64_t value_;
    uint64_t maximum_;
    uint64_t valueIdx_;
    uint64_t maximumIdx_;
    uint64_t valueTotal_;
    uint64_t maximumTotal_;

    uint64_t offsetHeaderEnd_;    // From version major.minor
    uint64_t offsetHeaderEndOut_; // From version major.minor
    uint64_t offsetPointsStart_;
    uint64_t offsetPointsStartOut_;
    uint64_t offsetPointsEnd_;
    uint64_t offsetPointsEndOut_;
    uint64_t sizePoints_;
    uint64_t sizePointsOut_;
    uint64_t sizeFile_;
    uint64_t sizeFileOut_;
    size_t sizePoint_;
    size_t sizePointOut_;
    size_t sizePointFormat_;

    Box<double> boundary_;

    uint32_t rgbMax_;
    uint32_t intensityMax_;

    uint64_t start_;
    uint64_t current_;
    uint64_t max_;
    uint64_t step_;

    FileIndex indexMain_;
    FileIndex indexNode_;
    std::map<const FileIndex::Node *, uint64_t> indexMainUsed_;
    FileChunk indexFile_;

    FileLas inputLas_;
    FileLas outputLas_;
    std::string inputPath_;
    std::string outputPath_;
    std::string readPath_;
    std::string writePath_;

    // Settings
    FileIndexBuilder::Settings settings_;

    // Buffers
    std::vector<uint8_t> buffer_;
    std::vector<uint8_t> bufferOut_;
    std::vector<double> coords_;

    void openFiles();

    void nextState();
    void stateCopy();
    void stateCopyPoints();
    void stateMove();
    void stateMainBegin();
    void stateMainInsert();
    void stateMainEnd();
    void stateMainSort();
    void stateNodeBegin();
    void stateNodeInsert();
    void stateNodeEnd();
    void stateEnd();

    void formatPoint(uint8_t *pout, const uint8_t *pin) const;
};

#endif /* FILE_INDEX_BUILDER_HPP */
