There's a bit configuration that needs to be done in order to have debugging with ST-Link
-Connect ST-Link as well as the Internal USB, put daisy in DFU program mode, then run through this
-Also make sure you've selected the right configuration in Coretx Debug extension
-Regularly you need to put daisy in DFU program mode in order to upload new builds, so the only way to do this is to reset the power and call the daisy::System::ResetToBootloader(daisy::System::BootloaderMode::DAISY_INFINITE_TIMEOUT) if there's no access to the buttons on daisy's board

https://forum.electro-smith.com/t/out-of-flash-memory-walkthrough-with-samples/4370

*DFU program mode -> Breathing boot LED