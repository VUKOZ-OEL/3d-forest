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

/** @file IndexFileBuilder.hpp */

#ifndef INDEX_FILE_BUILDER_HPP
#define INDEX_FILE_BUILDER_HPP

// Include std.
#include <map>
#include <string>
#include <vector>

// Include 3D Forest.
#include <ChunkFile.hpp>
#include <IndexFile.hpp>
#include <LasFile.hpp>

// Include local.
#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Index File Builder. */
class EXPORT_EDITOR IndexFileBuilder
{
public:
    /** Index File Builder Settings. */
    class EXPORT_EDITOR Settings
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

    IndexFileBuilder();
    ~IndexFileBuilder();

    void start(const std::string &outputPath,
               const std::string &inputPath,
               const IndexFileBuilder::Settings &settings);

    void next();

    bool end() const { return state_ == STATE_NONE; }

    double percent() const;

    static std::string extension(const std::string &path);

    static void index(const std::string &outputPath,
                      const std::string &inputPath,
                      const IndexFileBuilder::Settings &settings);

protected:
    // Settings.
    IndexFileBuilder::Settings settings_;

    /** Index File Builder State. */
    enum State
    {
        // Clean state. All files are open.
        STATE_NONE,

        // Files are open. Output file has header. Buffers are configured.
        STATE_BEGIN,

        // Prepare initial attributes.
        STATE_CREATE_ATTRIBUTES,

        // Copy VLR file data.
        STATE_COPY_VLR,

        // Copy point and attribute file data.
        STATE_COPY_POINTS,

        // Copy EVLR file data.
        STATE_COPY_EVLR,

        // Swap input and output files.
        STATE_MOVE,

        // Copy whole Las file.
        STATE_COPY,

        // Copy all attributes file.
        STATE_COPY_ATTRIBUTES,

        // Prepare index.
        STATE_MAIN_BEGIN,

        // Insert points to index.
        STATE_MAIN_INSERT,

        // Write main index.
        STATE_MAIN_END,

        // Distribute points to nodes.
        STATE_MAIN_SORT,

        // Sort points in each index page.
        STATE_NODE_INSERT,

        // Write index file.
        STATE_NODE_END,

        // Cleanup and create the final output file.
        STATE_END
    };

    // State.
    State state_;

    uint64_t value_;
    uint64_t maximum_;
    uint64_t valueIndex_;
    uint64_t maximumIndex_;
    uint64_t valueTotal_;
    uint64_t maximumTotal_;

    // Las files.
    LasFile inputLas_;
    LasFile outputLas_;

    std::string inputPath_;
    std::string outputPath_;
    std::string readPath_;
    std::string writePath_;

    // Las file buffers.
    std::vector<uint8_t> buffer_;
    std::vector<uint8_t> bufferOut_;

    // Las data.
    Box<double> boundary_;
    uint32_t rgbMax_;
    uint32_t intensityMax_;

    // File and point data.
    uint64_t offsetHeaderEnd_;
    uint64_t offsetHeaderEndOut_;
    uint64_t offsetPointsStart_;
    uint64_t offsetPointsStartOut_;
    uint64_t offsetPointsEnd_;
    uint64_t offsetPointsEndOut_;

    uint64_t nPoints_;
    uint64_t sizePoints_;
    uint64_t sizePointsOut_;
    uint64_t sizeFile_;
    uint64_t sizeFileOut_;

    size_t sizePoint_;
    size_t sizePointOut_;
    size_t sizePointFormat_;

    uint64_t copyPointsRestartIndex_;
    uint64_t copyPointsCurrentIndex_;
    uint64_t copyPointsSkipCount_;

    // Attributes.
    uint64_t sizeOfAttributesPerPoint_;
    uint64_t sizeOfAttributes_;
    LasFile::AttributesBuffer attributes_;
    LasFile::AttributesBuffer attributesOut_;

    // Index.
    IndexFile indexMain_;
    IndexFile indexNode_;
    ChunkFile indexFile_;
    std::map<const IndexFile::Node *, uint64_t> indexMainUsed_;
    std::vector<double> coords_;

    void openFiles();

    void nextState();
    void stateCreateAttributes();
    void stateCopy();
    void stateCopyPoints();
    void stateCopyAttributes();
    void stateMove();
    void stateMainBegin();
    void stateMainInsert();
    void stateMainEnd();
    void stateMainSort();
    void stateNodeInsert();
    void stateNodeEnd();
    void stateEnd();

    void formatPoint(uint8_t *pout, const uint8_t *pin) const;
};

#include <WarningsEnable.hpp>

#endif /* INDEX_FILE_BUILDER_HPP */
