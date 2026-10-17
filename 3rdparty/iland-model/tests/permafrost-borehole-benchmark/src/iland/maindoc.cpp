/** \mainpage Welcome to the iLand Source Code Documentation!

\section intro_sec Introduction

\htmlonly
<img src="https://iland-model.org/img/tiki/iland/iland_gfx1.png" style="float:right"></br>
<img src="https://iland-model.org/img/tiki/iland/iLand_gfx6.png" style="float:right">
\endhtmlonly

iLand is a model of forest landscape dynamics, simulating individual tree competition, growth, mortality, and regeneration.
It addresses interactions between climate (change), disturbance regimes, vegetation dynamics, and forest management.


iLand is a research tool already used by a growing number of scientists in the US and Europe.
The iLand software is released under the GNU General Public License and freely available.


The iLand wiki with 120+ pages of documentation, links to the download package and much more is located <a href="https://iland-model.org">here.</a>

\section About the source code

Qt toolkit

iLand uses the cross platform C++ toolkit Qt. <a href="http://qt-project.org/">Qt</a> is available for Windows, Linux and Mac platforms (along several mobile platforms like Symbian, Maemo, and others) - so you are not locked in with regard to operating systems. The Qt framework is open source and offers lots of functionality beyond the graphical user interface.


Database

iLand uses the SQLite (http://www.sqlite.org/) database. This is a leight-weight open source database that requires no installation, is cross platform and has the reputation to be very fast.
SQLite stores the all tables of a database within one single file. This approach could get quite handy for the storage of model outputs.

Subversion

The iLand source code is stored in a <a href="http://subversion.tigris.org/">subversion</a> source code repository. The public SVN-repository (through WebSVN) is available on the net: <a href="https://iland-model.org/publicsvn/">https://iland-model.org/publicsvn/</a>. Note that there is also a "private" SVN for portions of the model that are not published yet.



 */

/** @defgroup core iLand core classes
  This module contains more/less core model of iLand.
  The class Model is the top level container of iLand. The Model holds a collection of ResourceUnits, links to SpeciesSet and Climate.
  ResourceUnit are grid cells with (currently) a size of 1 ha (100x100m). Many stand level processes (NPP produciton, WaterCycle) operate on this
  level.
  The Model also contain the landscape-wide 2m LIF-grid (https://iland-model.org/competition+for+light).

  The basic simulation entity of iLand is the individual Tree. Trees live on a ResourceUnit (i.e. trees are stored in lists owned by ResourceUnits).



 */

/** @defgroup tools iLand tools

 */
/** @defgroup script iLand scripting
The iLand classes and APIs for scripting (i.e.\ JavaScript) are described on this page.
 */

/** \page Javascript Objects

 Some general intro to javascript in iLand

- \subpage globals Object in global space
- \subpage climateconverter ClimateConverter
- \subpage csvfile CSVFile


Now you can proceed to the \ref advanced "advanced section".

*/

/** @defgroup GUI elements necessary for the iLand user interface


  */
