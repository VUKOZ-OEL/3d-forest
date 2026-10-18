iLand - the individual-based forest landscape and disturbance model
===================================================================

iLand is a forest landscape model and simulates how individual trees grow, compete, die, and regenerate under various climate, disturbance, and management scenarios.
Spanning scales from watersheds to landscapes (10,000+ hectares), iLand projects forest dynamics over decades and centuries, estimating species composition, stand structure, carbon storage, 
and potential impacts of climate change on disturbances and ecosystem services. 

Used by a growing research community, this open-source tool (GNU GPL) is hosted at the Technical University of Munich but employed by research groups worldwide. The model
has been steadily developed over the last 15 years and has been used on in more than 40 peer reviewed research papers.

## Resources

* Offical home page: https://iland-model.org
* Scientifc publications: https://iland-model.org/iLand+publications
* Discord-community-channel: https://discord.gg/e5trneUKav or https://tinyurl.com/iland-discord

## About this version
The original iLand is a C++ application that uses the Qt toolkit.
This version is built as a shared library and does not require Qt.
The library exposes a single function runilandmodel to run the iLand model.
