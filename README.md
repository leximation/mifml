# MIFML

After years of ignoring this code, and with the encouragement of a few interested colleagues, I'm making the source code and DTD for my MIFML conversion utility available to the community. The DTD was last updated in 2007, and the EXE was last updated in 2008, so this is all rather ancient. The basic purpose of MIFML is to allow the conversion of a FrameMaker MIF file into XML, which can then be processed with XSLT and published or then reconverted back to MIF and opened in FrameMaker. The MIFML utility is just a command line tool that converts from MIF to MIFML, and from MIFML to MIF.

The process of converting a MIF file to MIFML is a fairly straight forward process. The MIF statement name becomes the XML element name and the value of the statement becomes the value  of the 'value' attribute. Statements that define a group, wrap the child elements of that group. The content of all 'value' attributes is processed to convert invalid characters into the appropriate character entities ('>', '<', '&', '"'). The MIFML elements that diverge (as of FM7.0) from this general conversion process are: MIFFile, String, ImportObjectData, and DocFileInfo.

The utility itself just follows these basic rules to perform the conversion. There's currently no DTD validation (or much validation of any kind). The first step in bringing this tool up to date is to update the DTD to support new MIF statements. As long as the fundamental MIF statement structure hasn't changed, the EXE should still do the right thing and perform a useful conversion (both directions). We may find that some changes in the MIF statement structure don't work with the current logic, and the scary old C code will need to be revised.

A note about the "scary old C code" .. This was initially developed in 2003, with very little knowledge of proper coding techniques. Nothing is sacred here. If it should be trashed and rewritten, that's fine. 

I encourage interested parties to help with the analysis and updating of the DTD, and then to identify possible areas that need to be updated in the EXE.

We'll see where this ends up.

Cheers,
...scott
