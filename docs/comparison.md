# Solar Router Comparison

Several people asked me how YaSolR compares to other solutions, and I thought it would be useful to make a short comparison matrix to help people choose the solution that best fits their needs.

You can also have a look at the [benefits](benefits.md) page to read about all the advantages YaSolR has over other solutions.

I should note upfront that YaSolR being Open-Source, I am myself biased towards Open-Source solutions, but I will try to be as objective as possible in this comparison.

!!! warning

    If you think I made a mistake in this comparison, or misrepresented a solution, please let me know or send a pull request, with the corrections and link to the source, and I will update the comparison accordingly.

## Who am I

I am a professional software engineer with more than 20 years of experience in low level systems and concurrency.
As an experienced ESP32 / Arduino Core developer, I have made countless ESP32 projects and written several blog articles related to ESP32 development.
Some people also know me for my work on the [ESP32Async](https://github.com/ESP32Async) organization: the famous ESPAsyncWebServer library (Asynchronous Web Server).
I am specialized in solar routing / diversion and libraries around electricity and network management for ESP32 / Arduino.

In 2025, I was nominated twice as being one of the [top Arduino Library maintainer of the year](https://forum.arduino.cc/t/arduino-open-source-report-2025) ([PDF Report](https://mathieu.carbou.me/ArduinoOpenSourceReport2025.pdf)), both as individual and as part of the [ESP32Async](https://github.com/ESP32Async) organization for the work done on the Async WebServer libraries.

## Vocabulary

- **Phase Control** is a control algorithm that consists in controlling the phase angle at which the load is turned on, allowing for a more precise and reactive control of the power. It can also cause elevated harmonic current levels and requires additional measures to help reduce them.

- **Burst Fire** is a control algorithm that consists in turning the load on and off for several full or semi periods to achieve the desired power. It is innacurate compared to the other algorithms and can cause under certain circumstances some flickering. It is alos not very reactive as it relies on tracking the power through a fixed window or tables. Implementations often do not control the grid balancing, which can lead to DC components and a few harmonics (on semi periods). Burst Fire is often what's implemented when Phase Control is not used. I am calling it the "poor man's Cycle Stealing". **YaSolR is not using Burst Fire**.

- **Cycle Stealing** is a control algorithm that consists in stealing cycles from the grid to achieve the desired power, allowing for an immediate reaction to duty cycle changes and a better control of the grid balance to avoid DC components and reduce harmonics. The implementation of Cycle Stealing with delta-sigma modulation and Bresenham's algorithm allows for a very precise and efficient control of the power and does not have the drawbacks of Burst Fire. **YaSolR is using Cycle Stealing with delta-sigma modulation and Bresenham's algorithm**.

## Legend

- ✅: Yes, supported
- ❌: No, not supported
- ⚠️: Partially supported, or support depends on the user setup, or warnings apply

## YaSolR (reference project)

👉 [https://yasolr.carbou.me](https://yasolr.carbou.me)

- ✅ Highly experienced developer ([@mathieucarbou](https://github.com/mathieucarbou)) - About me [here](benefits.md#about-the-author)
- ✅ Open-Source ([link](https://github.com/mathieucarbou/YaSolR))
- ✅ Source code available and auditing possible ([link](https://github.com/mathieucarbou/YaSolR))
- ✅ Forum ([link](https://forum-photovoltaique.fr/viewtopic.php?t=76764))
- ✅ Discord ([link](https://discord.gg/jBTgweft7P))
- ✅ Facebook Group ([link](https://www.facebook.com))
- ✅ Bug Report System ([link](https://github.com/mathieucarbou/YaSolR/issues))
- ✅ Support System ([link](https://github.com/mathieucarbou/YaSolR/discussions))
- Documentation:
    - ✅ Available and technical ([link](https://yasolr.carbou.me))
    - ✅ Covers harmonics ([link](overview.md/#harmonics))
    - ✅ **Covers solutions to mitigate them** ([link](overview.md/#recommendations-to-reduce-harmonics-and-flickering))
    - ✅ **Mentions the CEI 61000-3-2 harmonic regulation and how to comply with it** ([link](overview.md/#harmonics))
- Setup:
    - ✅ Assembly: DIY (easy)
    - ✅ Installation: DIY
    - ✅ Monophase support
    - ✅ Three-phase support
- Routing algorithms:
    - ✅ **High-Resolution Phase Control**
        - ✅ High resolution of 1µs
        - ✅ Display Harmonic Current Levels
        - ✅ Software options to help reduce them
    - ✅ **Smart Cycle Stealing implementation (delta-sigma modulation with Bresenham's algorithm)** ([link](overview.md/#cycle-stealing-control))
        - ✅ Not window-limited, reacts immediately to duty cycle changes
        - ✅ Grid Balance Control to avoid DC components
- Dimmer Hardware:
    - ✅ Random SSR
    - ✅ TRIAC / RobotDyn
    - ✅ Synchronous SSR
    - ✅ **DAC-controlled voltage regulators** (LCTC, LSA, etc)
- Product sold:
    - ❌ No product sold
    - ⚠️ Up to the user to install according to regulations (including CEI 61000-3-2)

!!! tip

    YaSolR is the only Open-Source router correctly implementing Interrupt Routines.
    More explanation in the [benefits](benefits.md) page.

## Other Open-Source Solutions

As a reminder, Open-Source Solutions provide these benefits compared to commercial solutions:

- Lower cost compared to commercial solutions, as they are typically free to use and modify
- Greater flexibility and customization options, as users can modify the source code to fit their specific needs
- Community support and contributions, which can lead to faster development and bug fixes
- Transparency, as the source code is available for review and auditing, allowing users to verify the security and performance of the solution
- Potential for innovation, as open-source projects can benefit from contributions from a wide range of developers and organizations, leading to new features and improvements that may not be available in commercial solutions
- Ability to avoid vendor lock-in, as users can switch to another open-source solution or modify the existing one without being tied to a specific vendor's ecosystem
- Encourages collaboration and knowledge sharing within the community, fostering a culture of learning and improvement among developers and users alike
- Often has a larger and more active user base, which can lead to better support and resources available online, such as forums, documentation, and tutorials
- Documentation and resources are often freely available, making it easier for users to learn and implement the solution without incurring additional costs for training or support

### AASun-V2

👉 [https://github.com/Nikitarc/AASun-V2](https://github.com/Nikitarc/AASun-V2)

- ✅ Open-Source ([link](https://github.com/Nikitarc/AASun-V2))
- ✅ Source code available and auditing possible ([link](https://github.com/Nikitarc/AASun-V2))
- ✅ Forum ([link](https://forum-photovoltaique.fr/viewtopic.php?t=73878))
- ❌ No Discord
- ❌ No Facebook Group
- ✅ Bug Report System ([link](https://github.com/Nikitarc/AASun-V2/issues))
- ✅ Support System (through forum)
- Documentation:
    - ✅ Available and technical ([link](https://github.com/Nikitarc/AASun-V2/tree/main/Doc))
    - ❌ Does not cover harmonics
    - ❌ Does not cover solutions to mitigate them
    - ❌ Does not mention the CEI 61000-3-2 harmonic regulation and how to comply with it
- Setup:
    - ⚠️ Assembly: DIY and **complex** (PCB to be made)
    - ✅ Installation: DIY
    - ✅ Monophase support
    - ❌ Three-phase support
- Routing algorithms:
    - ✅ Phase Control
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ❌ No Cycle Stealing support
- Dimmer Hardware:
    - ✅ Random SSR
    - ❌ TRIAC / RobotDyn
    - ❌ Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ❌ No product sold
    - ⚠️ Up to the user to install according to regulations (including CEI 61000-3-2)

### APPER / Clyric

👉 [https://ota.apper-solaire.org/index.php](https://ota.apper-solaire.org/index.php)

- ✅ Open-Source ([link](https://github.com/xlyric))
- ✅ Source code available and auditing possible ([link1](https://github.com/xlyric/PV-discharge-Dimmer-AC-Dimmer-KIT-Robotdyn), [link2](https://github.com/xlyric/pv-router-esp32))
- ✅ Forum ([link](https://forum-photovoltaique.fr/viewtopic.php?f=110&t=41777#p476566))
- ❌ No Discord
- ❌ No Facebook Group
- ✅ Bug Report System ([link1](https://github.com/xlyric/PV-discharge-Dimmer-AC-Dimmer-KIT-Robotdyn/issues), [link2](https://github.com/xlyric/pv-router-esp32/issues))
- ✅ Support System (through forum)
- Documentation:
    - ✅ Available and technical ([link1](https://wiki.apper-solaire.org), [link2](https://pvrouteur.apper-solaire.org/shelves/fr-documentation))
    - ✅ Covers harmonics a little
    - ❌ Does not cover solutions to mitigate them
    - ❌ Does not mention the CEI 61000-3-2 harmonic regulation and how to comply with it
- Setup:
    - ✅ Assembly: DIY or pre-assembled PCB
    - ✅ Installation: DIY
    - ✅ Monophase support
    - ✅ Three-phase support
- Routing algorithms:
    - ✅ Phase Control
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ⚠️ Basic Cycle Stealing implementation
        - ⚠️ Limited to a fixed window (100 semi-periods)
        - ❌ No Grid Balance Control to avoid DC components
- Dimmer Hardware:
    - ✅ Random SSR
    - ✅ TRIAC / RobotDyn
    - ✅ Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ✅ Pre-assembled PCB
    - ❌ Not CE
    - ❌ Not NF
    - ⚠️ CEI 61000-3-2 compliance not guaranteed (there is no active circuit to reduce harmonic levels)
    - ⚠️ Up to the user to install according to regulations (including CEI 61000-3-2)

### EcoPV / MaxPV

👉 [https://github.com/Jetblack31/MaxPV](https://github.com/Jetblack31/MaxPV)

- ✅ Open-Source ([link](https://github.com/Jetblack31/MaxPV))
- ✅ Source code available and auditing possible ([link](https://github.com/Jetblack31/MaxPV))
- ✅ Forum ([link](https://forum-photovoltaique.fr/viewtopic.php?f=110&t=55244%C2%A0))
- ❌ No Discord
- ❌ No Facebook Group
- ✅ Bug Report System ([link](https://github.com/Jetblack31/MaxPV/issues))
- ✅ Support System (through forum)
- Documentation:
    - ⚠️ Poor ([link](https://github.com/Jetblack31/MaxPV))
    - ❌ Does not cover harmonics
    - ❌ Does not cover solutions to mitigate them
    - ❌ Does not mention the CEI 61000-3-2 harmonic regulation and how to comply with it
- Setup:
    - ⚠️ Assembly: DIY and **complex** (PCB to be made)
    - ✅ Installation: DIY
    - ✅ Monophase support
    - ❌ Three-phase support
- Routing algorithms:
    - ✅ Phase Control
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ⚠️ Burst Fire
        - ⚠️ Prone to flickering and inaccuracy
        - ⚠️ Limited reactivity due to the tracking through a fixed window / tables
        - ❌ No Grid Balance Control to avoid DC components
    - ❌ No Cycle Stealing support
- Dimmer Hardware:
    - ✅ Random SSR
    - ✅ TRIAC / RobotDyn
    - ✅ Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ❌ No product sold
    - ⚠️ Up to the user to install according to regulations (including CEI 61000-3-2)

### F1ATB

👉 [https://github.com/F1ATB/Solar-Router-F1ATB](https://github.com/F1ATB/Solar-Router-F1ATB)

- ✅ Open-Source ([link](https://github.com/F1ATB/Solar-Router-F1ATB))
- ✅ Source code available and auditing possible ([link](https://github.com/F1ATB/Solar-Router-F1ATB))
- ✅ Forum ([link](https://f1atb.fr/forum_f1atb/forum-4.html))
- ❌ No Discord
- ✅ Facebook Group ([link](https://www.facebook.com/groups/948056843630328))
- ✅ Bug Report System ([link](https://github.com/F1ATB/Solar-Router-F1ATB/issues))
- ✅ Support System (through forum)
- Documentation:
    - ✅ Available and technical ([link](https://f1atb.fr/fr/documentation-routeur-photovoltaique-f1atb/))
    - ✅ Covers harmonics a little
    - ❌ Does not cover solutions to mitigate them
    - ❌ Does not mention the CEI 61000-3-2 harmonic regulation and how to comply with it
- Setup:
    - ✅ Assembly: DIY or pre-assembled PCB
    - ✅ Installation: DIY
    - ✅ Monophase support
    - ✅ Three-phase support
- Routing algorithms:
    - ✅ Phase Control
        - ⚠️ Lower resolution of 100µs
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ⚠️ Burst Fire
        - ⚠️ Prone to flickering and inaccuracy
        - ⚠️ Limited reactivity due to the tracking through a fixed window / tables
        - ❌ No Grid Balance Control to avoid DC components
    - ⚠️ Basic Cycle Stealing implementation
        - ⚠️ Limited to a fixed window / tables
        - ⚠️ No strict Grid Balance Control to avoid DC components (not as good as Bresenham's algorithm)
- Dimmer Hardware:
    - ✅ Random SSR
    - ✅ TRIAC / RobotDyn
    - ✅ Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ❌ No product sold
    - ⚠️ Up to the user to install according to regulations (including CEI 61000-3-2)

### Mk2 PV Router

👉 [https://mk2pvrouter.com](https://mk2pvrouter.com)

- ✅ Open-Source ([link](https://github.com/FredM67/Mk2PVRouter))
- ✅ Source code available and auditing possible ([link](https://github.com/FredM67/Mk2PVRouter))
- ✅ Forum ([link](https://github.com/FredM67/PVRouter-3-phase/discussions))
- ❌ No Discord
- ✅ Facebook Group ([link](https://www.facebook.com/groups/3571488193062570/))
- ✅ Bug Report System ([link](https://github.com/FredM67/PVRouter-3-phase/issues))
- ✅ Support System (through forum)
- Documentation:
    - ✅ Available and technical ([link1](https://openenergymonitor.org/mk2pvrouter/), [link2](https://fredm67.github.io/Mk2PVRouter/))
    - ✅ Covers harmonics a little
- Setup:
    - ✅ Assembly: DIY or pre-assembled PCB
    - ✅ Installation: DIY
    - ✅ Monophase support
    - ✅ Three-phase support
- Routing algorithms:
    - ❌ Phase Control
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ⚠️ Burst Fire ([link](https://fredm67.github.io/Mk2PVRouter/burst-fire-control/))
        - ⚠️ Prone to flickering and inaccuracy
        - ⚠️ Limited reactivity due to the tracking through a fixed window / tables
        - ❌ No Grid Balance Control to avoid DC components
    - ❌ No Cycle Stealing support
- Dimmer Hardware:
    - ❌ Random SSR
    - ✅ TRIAC
    - ✅ Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ✅ Pre-assembled Components
    - ❌ Not CE
    - ❌ Not NF
    - ✅ CEI 61000-3-2 compliant
    - ⚠️ Up to the user to install according to regulations

### Tignous / Rolrider

👉 [https://forum-photovoltaique.fr/viewtopic.php?t=39159](https://forum-photovoltaique.fr/viewtopic.php?t=39159)

- ✅ Open-Source ([link](https://forum-photovoltaique.fr/viewtopic.php?t=39159))
- ✅ Source code available and auditing possible ([link](https://forum-photovoltaique.fr/viewtopic.php?t=39159))
- ✅ Forum ([link](https://forum-photovoltaique.fr/viewtopic.php?t=39159))
- ❌ No Discord
- ❌ No Facebook Group
- ❌ No Bug Report System
- ✅ Support System (through forum)
- Documentation:
    - ✅ Available and technical ([link](https://forum-photovoltaique.fr/viewtopic.php?t=39159))
    - ✅ Covers harmonics a little
    - ❌ Does not cover solutions to mitigate them
    - ❌ Does not mention the CEI 61000-3-2 harmonic regulation and how to comply with it
- Setup:
    - ⚠️ Assembly: DIY and **complex**
    - ✅ Installation: DIY
    - ✅ Monophase support
    - ❌ Three-phase support
- Routing algorithms:
    - ✅ Phase Control
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ❌ No Cycle Stealing support
- Dimmer Hardware:
    - ✅ Random SSR
    - ❌ TRIAC
    - ❌ Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ❌ No product sold
    - ⚠️ Up to the user to install according to regulations (including CEI 61000-3-2)

## Commercial Solutions

- Provides Warranty
- Unknown support: could be good, could be bad, depends on the company and the contract
- Complete pre-built solution, ready to be installed and used
- Higher cost compared to open-source solutions
- Less flexibility and customization options compared to open-source solutions
- Potential vendor lock-in, making it difficult to switch to another solution in the future
- Source code unavailable, so this is impossible to audit and verify the security and performance of the solution
- Often biased documentation, as it may be designed to promote the company's products and services rather than providing objective information about the solution
- Some key topics like harmonics and solutions often not covered in the documentation

### Arsun

👉 [https://arsun-concept.com/arsun.html](https://arsun-concept.com/arsun.html)

- ❌ Not Open-Source
- ❌ No Source code available and no auditing possible
- ✅ Forum ([link](https://arsun-concept.com/forum/))
- ❌ No Discord
- ❌ No Facebook Group
- ❌ No Bug Report System
- ✅ Support System (through forum)
- Documentation:
    - ❌ Does not cover harmonics
    - ❌ Does not cover solutions to mitigate them
    - ❌ Does not mention the CEI 61000-3-2 harmonic regulation and how to comply with it
- Setup:
    - ✅ Assembly: pre-assembled
    - ✅ Installation: professional or self-installation possible
    - ✅ Monophase support
    - ✅ Three-phase support
- Routing algorithms:
    - ✅ Phase Control
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ❌ No Cycle Stealing support
- Dimmer Hardware:
    - ⚠️ TRIAC only
    - ❌ No support for Random SSR
    - ❌ No support for Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ✅ Pre-assembled product
    - ❌ Not CE
    - ❌ Not NF
    - ❌ CEI 61000-3-2 compliance not guaranteed (there is no active circuit to reduce harmonic levels)
    - ❌ No documentation on how to install according to regulations (including CEI 61000-3-2)

### eddi de Myenergi

👉 [https://www.myenergi.com/nl/fr/product/eddi/](https://www.myenergi.com/nl/fr/product/eddi/)

- ❌ Not Open-Source (**and very expensive**)
- ❌ No Source code available and no auditing possible
- ✅ Forum ([link](https://support.myenergi.com/hc/en-gb/categories/360003646857-eddi))
- ❌ No Discord
- ❌ No Facebook Group
- ✅ Bug Report System ([link](https://www.myenergi.com/nl/fr/product-support/eddi/))
- ⚠️ Support System unknown
- ⚠️ Documentation:
    - ✅ User Manual ([link](https://www.myenergi.com/nl/wp-content/uploads/sites/6/2023/03/B.-eddi_Manuel-dutilisation-1.pdf))
    - ✅ Installation Manual ([link](https://www.myenergi.com/nl/wp-content/uploads/sites/6/2023/03/A.-eddi_Manuel-dinstallation.pdf))
    - ⚠️ Technical Documentation not found
- Setup:
    - ✅ Assembly: pre-assembled
    - ✅ Installation: professional or self-installation possible
    - ✅ Monophase support
    - ✅ Three-phase support
- Routing algorithms:
    - ⚠️ Technical Documentation not found
- Dimmer Hardware:
    - ⚠️ Technical Documentation not found
- Product sold:
    - ✅ Pre-assembled product
    - ✅ CE
    - ✅ CEI 61000-3-2 compliant
    - ❌ Not NF

### EKOSIA

👉 [https://ekosia.fr/routeur-solaire-ekosia/](https://ekosia.fr/routeur-solaire-ekosia/)

- ❌ Not Open-Source (**and very expensive**)
- ❌ No Source code available and no auditing possible
- ❌ No Forum
- ❌ No Discord
- ❌ No Facebook Group
- ❌ No Bug Report System
- ❌ No Support System
- ⚠️ Documentation:
    - ⚠️ Technical Documentation not found
- Setup:
    - ✅ Assembly: pre-assembled
    - ✅ Installation: professional or self-installation possible
    - ✅ Monophase support
    - ✅ Three-phase support
- Routing algorithms:
    - ⚠️ Technical Documentation not found
- Dimmer Hardware:
    - ⚠️ Technical Documentation not found
- Product sold:
    - ✅ Pre-assembled product
    - ✅ CE
    - ⚠️ CEI 61000-3-2 compliance not guaranteed (there is no active circuit to reduce harmonic levels)
    - ❌ Not NF
    - ❌ No documentation on how to install according to regulations (including CEI 61000-3-2)

### MSUNPV

👉 [https://ard-tek.com](https://ard-tek.com)

- ❌ Not Open-Source
- ❌ No Source code available and no auditing possible
- ✅ Forum ([link](https://ard-tek.com/index.php/forum))
- ❌ No Discord
- ❌ No Facebook Group
- ❌ No Bug Report System
- ✅ Support System (through forum)
- Documentation:
    - ❌ Does not cover harmonics
    - ❌ Does not cover solutions to mitigate them
    - ❌ Does not mention the CEI 61000-3-2 harmonic regulation and how to comply with it
- Setup:
    - ✅ Assembly: pre-assembled
    - ✅ Installation: professional or self-installation possible
    - ✅ Monophase support
    - ❌ Three-phase support
- Routing algorithms:
    - ✅ Phase Control
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ❌ No Cycle Stealing support
- Dimmer Hardware:
    - ✅ Random SSR
    - ❌ TRIAC
    - ❌ Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ✅ Pre-assembled product
    - ❌ Not CE
    - ❌ Not NF
    - ❌ CEI 61000-3-2 compliance not guaranteed (there is no active circuit to reduce harmonic levels)
    - ❌ No documentation on how to install according to regulations (including CEI 61000-3-2)

### SMH-Tech

👉 [https://smhtech.fr/routeur-solaire/52-routeur-denergie-solaire-photovoltaique-monophase-triphase-modbus.html](https://smhtech.fr/routeur-solaire/52-routeur-denergie-solaire-photovoltaique-monophase-triphase-modbus.html)

- ❌ Not Open-Source
- ❌ No Source code available and no auditing possible
- ❌ No Forum
- ❌ No Discord
- ❌ No Facebook Group
- ❌ No Bug Report System
- ❌ No Public Support System (i.e. through forum)
- Documentation:
    - ✅ Installation manual only ([link](https://smhtech.fr/routeur-solaire/66-routeur-d-energie-solaire-automatique.html))
    - ✅ Covers harmonics a little
    - ❌ Does not cover solutions to mitigate them
    - ❌ Does not mention the CEI 61000-3-2 harmonic regulation and how to comply with it
- Setup:
    - ✅ Assembly: pre-assembled
    - ✅ Installation: professional or self-installation possible
    - ✅ Monophase support
    - ✅ Three-phase support
- Routing algorithms:
    - ✅ Phase Control
        - ❌ No display of Harmonic Current Levels
        - ❌ No Software option to help reduce them
    - ⚠️ Burst Fire on full and semi-periods
        - ⚠️ Prone to flickering and inaccuracy
        - ⚠️ Limited reactivity due to the tracking through a fixed window / tables
        - ❌ No Grid Balance Control to avoid DC components
    - ❌ No Cycle Stealing support
- Dimmer Hardware:
    - ⚠️ TRIAC only, with active cooling (fan). Documentation does not explain what is the behaviour when the fan is not working.
    - ❌ No support for Random SSR
    - ❌ No support for Synchronous SSR
    - ❌ No support for DAC-controlled voltage regulators (LCTC, LSA, etc)
- Product sold:
    - ✅ Pre-assembled product
    - ✅ Not CE
    - ❌ Not NF
    - ❌ CEI 61000-3-2 compliance not guaranteed (there is no active circuit to reduce harmonic levels)
    - ❌ No documentation on how to install according to regulations (including CEI 61000-3-2)

### Solar iBoost+

👉 [https://www.wattuneed.com/fr/chauffages-et-climatisations/4616-solar-iboost-0712971136489.html](https://www.wattuneed.com/fr/chauffages-et-climatisations/4616-solar-iboost-0712971136489.html)

- ❌ Not Open-Source (**and very expensive**)
- ❌ No Source code available and no auditing possible
- ❌ No Forum
- ❌ No Discord
- ❌ No Facebook Group
- ❌ No Bug Report System
- ⚠️ Support System unknown
- ⚠️ Documentation:
    - ❌ No User Manual
    - ✅ Installation Manual ([link](https://www.wattuneed.com/fr/chauffages-et-climatisations/4616-solar-iboost-0712971136489.html))
    - ⚠️ Technical Documentation not found
- Setup:
    - ✅ Assembly: pre-assembled
    - ✅ Installation: professional or self-installation possible
    - ✅ Monophase support
    - ❌ Three-phase support
- Routing algorithms:
    - ⚠️ Technical Documentation not found
- Dimmer Hardware:
    - ⚠️ Technical Documentation not found
- Product sold:
    - ✅ Pre-assembled product
    - ✅ CE
    - ✅ CEI 61000-3-2 compliant (apparently)
    - ❌ Not NF
