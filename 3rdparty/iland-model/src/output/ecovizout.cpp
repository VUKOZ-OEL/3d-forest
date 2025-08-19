#include "ecovizout.h"
#include "model.h"
#include "resourceunit.h"
#include "tree.h"
#include "stamp.h"
#include "species.h"

#include <iostream>
#include <fstream>

EcoVizOut::EcoVizOut()
{
    setName("tree output for visualization software", "ecoviz");
    setDescription("This is a special output for linking with the visualization tool 'Ecoviz'.\n "\
                   "The output is a small standard iLand outpunt and a special textfile ('PDB') " \
                   "which contains *all* trees and *all* saplings of a year (i.e. it tends to get big!). " \
                   "Provide a file pattern in 'fileName', a $-sign is replaced with the current year. For example " \
                   "output/ecoviz_$.pdb is saved as output/ecoviz_0.pdb (initial state), output/ecoviz_1.pdb (after 1 year of simulation), ....). \n" \
                   "You can use the 'condition' to control if the output should be created for the current year(see also dynamic stand output)");
    columns() << OutputColumn::year()
              << OutputColumn("count_trees", "total number of trees count saved to file", OutInteger)
              << OutputColumn("count_saplings", "total number saplings saved to file", OutInteger)
              << OutputColumn("filename", "filename of the created output PDB file", OutString);

}

void EcoVizOut::exec()
{
    Model *m = GlobalSettings::instance()->model();
    if (!mCondition.isEmpty())
        if (!mCondition.calculateBool(GlobalSettings::instance()->currentYear()))
            return;
    int total_tree_count = 0;
    int total_cohort_count = 0;
    foreach (const ResourceUnit *ru,  m->ruList()) {
        total_tree_count += static_cast<int>(ru->statistics().count());
        total_cohort_count += ru->statistics().cohortCount();
    }

    QString stryear = QString::number(currentYear());
    QString file = mFilePattern;
    file.replace("$", stryear);

    *this << currentYear()
            << total_tree_count
            << total_cohort_count
            << file;
    writeRow();

    // write the actual file
    file = GlobalSettings::instance()->path(file);

    if (mBinaryMode) {
        writePDBBinaryFile(file, total_tree_count, total_cohort_count, currentYear());
        qDebug() << "Saved (binary) PDB file" << file;

    } else {
        writePDBFile(file, total_tree_count, total_cohort_count, currentYear());
        qDebug() << "Saved (text-based) PDB file" << file;
    }

}

void EcoVizOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);

    mBinaryMode = settings().valueBool(".binary", "false");

    mFilePattern = settings().value(".fileName", "output/pdb_$.pdb");

}

bool EcoVizOut::writePDBFile(QString fileName, int n_trees, int n_cohorts, int year)
{
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        // open a stream
        QTextStream stream(&file);
        QChar ws = ' ';

        //stream << "Center Point: " << iter_result[0] << "  " << iter_result[1]
        //       << "  " << iter_result[2] << " Rotation: " << iter_result[3] <<'\n';
        stream << "3.0" << Qt::endl; // PDB_file_version
        // version 3.0: include local origin in metric coordinates
        stream << GlobalSettings::instance()->settings().value("model.world.location.x") << " "
               << GlobalSettings::instance()->settings().value("model.world.location.y") << Qt::endl;
        stream << year << Qt::endl; // Time_step_number
        qint64 ntree_pos = stream.pos();
        stream << "          " << Qt::endl; // space for total number of trees

        // now loop over all trees and write a line for each tree
        AllTreeIterator at(GlobalSettings::instance()->model());
        n_trees = 0;
        while (Tree *tree = at.next()) {
            // Tree_ID<int> species_ID<cahr> pos_x<float> pos_y<float> height<float> canopy_radius<float> diameter_at_breast_height<float> status<int>
            stream << tree->id() << ws << tree->species()->id() << ws <<
                      tree->position().x() << ws << tree->position().y() << ws << tree->height() << ws;
            stream << tree->stamp()->reader()->crownRadius() << ws;
            stream << tree->dbh() << ws << (tree->isDead() ? 1 : 0);
            stream << Qt::endl;
            ++n_trees;
        }

        // now loop over all the saplings
        qint64 nsap_pos = stream.pos();
        stream << "          " << Qt::endl;
        n_cohorts = 0;
        Saplings *saplings = GlobalSettings::instance()->model()->saplings();
        foreach(ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
            if (ru->id()==-1)
                continue; // do not include if out of project area


            SaplingCell *s = ru->saplingCellArray();
            for (int px=0;px<cPxPerHectare;++px, ++s) {
                int n_on_px = s->n_occupied();
                if (n_on_px>0) {

                    QPointF coord = saplings->coordOfCell(ru, px);

                    for (int i=0;i<NSAPCELLS;++i) {
                        if (s->saplings[i].is_occupied()) {
                            ResourceUnitSpecies *rus = s->saplings[i].resourceUnitSpecies(ru);
                            const Species *species = rus->species();
                            double dbh = s->saplings[i].height / species->saplingGrowthParameters().hdSapling  * 100.;
                            // check minimum dbh
                            if (dbh < 0.1)
                                continue;

                            double n_repr = species->saplingGrowthParameters().representedStemNumberH(s->saplings[i].height) / static_cast<double>(n_on_px);


                            stream << coord.x() << ws << coord.y() << ws <<
                                      rus->species()->id() << ws <<
                                      dbh << ws <<
                                      s->saplings[i].height << ws <<
                                      n_repr << Qt::endl;
                            ++n_cohorts;

                        }
                    }
                }
            }
        }


        // write back the number of trees / saplings in the file
        file.seek(ntree_pos);
        QString nstr = QString::number(n_trees);
        file.write(nstr.toUtf8());

        file.seek(nsap_pos);
        nstr = QString::number(n_cohorts);
        file.write(nstr.toUtf8());

        file.close();

        return true;

    } else {
        qDebug() << "writePDBFile: Error opening file" << fileName;
        return false;
    }
}

bool EcoVizOut::writePDBBinaryFile(QString fileName, int n_trees, int n_cohorts, int year)
{

    QString binary_file_name = fileName.append("b");
    std::ofstream ofs;
    ofs.open(binary_file_name.toLocal8Bit().constData(), std::ios::binary);
    if (!ofs.is_open())
        throw IException("EcoVizOut: could not open file " + binary_file_name );

    const std::string version_string = "3.0";

    int64_t world_location_x = GlobalSettings::instance()->settings().valueDouble("model.world.location.x");
    int64_t world_location_y = GlobalSettings::instance()->settings().valueDouble("model.world.location.y");


    // char vector of species names
    std::vector<std::array<char, 4> > species_names;
    for (auto *s : GlobalSettings::instance()->model()->speciesSet()->activeSpecies()) {
        if (s->index() != static_cast<int>(species_names.size()))
            throw IException("aarrrghhh");

        QByteArray species_id = s->id().toUtf8();

        std::array<char, 4> temp_array; // Create a temporary array and set to 0
        std::memset(temp_array.data(), 0, 4);

        size_t copySize = std::min(species_id.size(), static_cast<qsizetype>(4));
        std::memcpy(temp_array.data(), species_id.constData(), copySize);

        species_names.push_back(temp_array); // Copy the *contents* of temp_array    }

    }

    struct cohortA {
        int treeid;
        char code[4]; // 4 byte ASCII tree code
        int x;
        int y;
        float height;
        float radius;
        float dbh;
        int dummy;
    };

    std::vector<cohortA> cohortAdata;

    // see how many trees we'll have
    AllTreeIterator at(GlobalSettings::instance()->model());
    n_trees = 0;
    while (Tree *tree = at.next()) {
        ++n_trees;
    }

    // reserve a big chunk of RAM and fill the data from
    // the trees in iLand
    cohortAdata.resize(n_trees);
    long nBytes = sizeof(cohortA) * n_trees;

    AllTreeIterator at2(GlobalSettings::instance()->model());
    int i = 0;
    while (Tree *tree = at2.next()) {
        cohortA &ca = cohortAdata[i];
        // use the lookup table (with 0-padded 4 character strings)
        std::memcpy(ca.code, &species_names[tree->species()->index()], 4);
        ca.x = tree->position().x();
        ca.y = tree->position().y();
        ca.height = tree->height();
        ca.radius = tree->stamp()->reader()->crownRadius();
        ca.dbh = tree->dbh();
        ca.dummy = (tree->isDead() ? 1 : 0);

        ++i;
        if (i>n_trees)
            throw IException("ecoviz out: number of trees wrong!");
    }


    int slen = version_string.length();
    //cout << "Version string: " << versionNumber << " of length " << slen << endl;
    ofs.write(reinterpret_cast<const char*>(&slen), sizeof(int));
    ofs.write(version_string.c_str(), slen); // don't store null
    ofs.write(reinterpret_cast<const char*>(&world_location_x), sizeof(int64_t));
    ofs.write(reinterpret_cast<const char*>(&world_location_y), sizeof(int64_t));
    ofs.write(reinterpret_cast<const char*>(&year), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&n_trees), sizeof(int));
    // Write the big data block
    ofs.write(reinterpret_cast<const char*>(cohortAdata.data()), nBytes);

    if (!ofs)
        throw IException("Something went wrong when writing first part of " + binary_file_name);

    qDebug()<<  "Number of bytes in part A of binary file: " << nBytes << "(" << n_trees << " trees)";
    cohortAdata.clear();

    // now the saplings


    struct cohortB {
        int xs;
        int ys;
        char code[4];
        float dbh;
        float height;
        float nplants;
    };

    std::vector<cohortB> cohortBdata;
    n_cohorts = 0;
    // count saplings (this means to loop once over all the data....)
    foreach(ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
        if (ru->id()==-1)
            continue; // do not include if out of project area
        SaplingCell *s = ru->saplingCellArray();
        for (int px=0;px<cPxPerHectare;++px, ++s) {
            int n_on_px = s->n_occupied();
            if (n_on_px>0) {
                for (int i=0;i<NSAPCELLS;++i) {
                    if (s->saplings[i].is_occupied()) {
                        ResourceUnitSpecies *rus = s->saplings[i].resourceUnitSpecies(ru);
                        const Species *species = rus->species();
                        double dbh = s->saplings[i].height / species->saplingGrowthParameters().hdSapling  * 100.;
                        // check minimum dbh
                        if (dbh >= 0.1)
                            ++n_cohorts;
                    }
                }
            }
        }
    }

    cohortBdata.resize(n_cohorts);
    // now fill the structure
    Saplings *saplings = GlobalSettings::instance()->model()->saplings();
    cohortB *cb = &cohortBdata[0];
    int n_filled = 0;
    foreach(ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
        if (ru->id()==-1)
            continue; // do not include if out of project area

        SaplingCell *s = ru->saplingCellArray();
        for (int px=0;px<cPxPerHectare;++px, ++s) {
            int n_on_px = s->n_occupied();
            if (n_on_px>0) {

                QPointF coord = saplings->coordOfCell(ru, px);

                for (int i=0;i<NSAPCELLS;++i) {
                    if (s->saplings[i].is_occupied()) {
                        ResourceUnitSpecies *rus = s->saplings[i].resourceUnitSpecies(ru);
                        const Species *species = rus->species();
                        double dbh = s->saplings[i].height / species->saplingGrowthParameters().hdSapling  * 100.;
                        // check minimum dbh
                        if (dbh < 0.1)
                            continue;

                        double n_repr = species->saplingGrowthParameters().representedStemNumberH(s->saplings[i].height) / static_cast<double>(n_on_px);

                        auto *species_char = &species_names[rus->species()->index()];
                        if (species_char->front() == '\0')
                            throw IException("Ecoviz export - invalid species");
                        cb->xs = coord.x();
                        cb->ys = coord.y();
                        std::memcpy(cb->code, species_char, 4);
                        cb->dbh = dbh;
                        cb->height = s->saplings[i].height;
                        cb->nplants = n_repr;
                        cb++;

                        if (++n_filled > n_cohorts)
                            throw IException("Ecovizout: invalid number of saplings!");

                    }
                }
            }
        }
    }

    long nBytesB = sizeof(cohortB)*cohortBdata.size();
    qDebug()<<  "Number of bytes in part B of binary file: " << nBytesB << "(" << n_cohorts << " cohorts)";
    ofs.write(reinterpret_cast<const char*>(&n_cohorts), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(cohortBdata.data()), nBytesB);
    if (!ofs)
        throw IException("EcovizOut:Something went wrong when writing second part of " + binary_file_name);
    ofs.close();

    qDebug() << "Wrote total of (partA = " << nBytes << " and partB = " << nBytesB << ") - total: " <<
        (nBytes+nBytesB) << " bytes";

    return true;
}
