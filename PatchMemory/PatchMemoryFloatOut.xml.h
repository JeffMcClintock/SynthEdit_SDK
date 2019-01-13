// File genereated by XmlToHeaderFile.exe from module xml definition.
const char* PATCHMEMORYFLOATOUT_XML = 
"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"

"<PluginList>"
  "<Plugin id=\"SE PatchMemory Float Out\"  name=\"PatchMemory Float Out3\" category=\"Sub-Controls\" helpUrl=\"Patchmemory.htm\" >"
    "<Parameters>"
      "<Parameter id=\"0\" datatype=\"float\" direction=\"out\"/>"
    "</Parameters>"

    "<Audio>"
      "<Pin id=\"0\" name=\"Value In\" direction=\"in\" datatype=\"float\"  />"
      "<Pin id=\"1\" name=\"PM Value Out\" direction=\"out\" datatype=\"float\" parameterId=\"0\" private =\"true\" autoConfigureParameter=\"true\"/>"
    "</Audio>"

    "<GUI>"
      "<Pin id=\"0\" name=\"PM Value In\" direction=\"in\" datatype=\"float\" parameterId=\"0\" />"
      "<Pin id=\"1\" name=\"PM Name In\" direction=\"in\" datatype=\"string\" parameterId=\"0\" parameterField=\"ShortName\"/>"
      "<Pin id=\"2\" name=\"PM Menu Items\" direction=\"in\" datatype=\"string\" parameterId=\"0\" parameterField=\"MenuItems\"/>"
      "<!-- Little unusual, this is an 'in' even though it's purpose is to send to host-->"
      "<Pin id=\"3\" name=\"PM Menu Selection\" direction=\"in\" datatype=\"int\" parameterId=\"0\" parameterField=\"MenuSelection\"/>"
      "<Pin id=\"4\" name=\"AnimationIn\" direction=\"in\" datatype=\"float\" parameterId=\"0\" parameterField=\"Normalized\"/>"

      "<Pin id=\"5\" name=\"Name\" direction=\"out\" datatype=\"string\" />"
      "<Pin id=\"6\" name=\"Value\" direction=\"out\" datatype=\"float\" />"
      "<!-- legacy only. for replacing the Patch Mem Float Out - B. minimised here to hide it. -->"
      "<Pin id=\"7\" name=\"Value Out\" direction=\"in\" datatype=\"float\" isMinimised=\"true\"/>"
      "<Pin id=\"8\" name=\"Animation Position\" direction=\"out\" datatype=\"float\" />"
      "<Pin id=\"9\" name=\"Menu Items\" direction=\"out\" datatype=\"string\"/>"
      "<Pin id=\"10\" name=\"Menu Selection\" direction=\"out\" datatype=\"int\"/>"
    "</GUI>"
  "</Plugin>"

  "<!-- legacy only. for replacing the Patch Mem Float Out - B. category 'Debug' to hide it on module list. -->"
  "<Plugin id=\"SE PatchMemory Float Out B2\"  name=\"PatchMemory Float Out B2\" category=\"Debug\" >"
    "<Parameters>"
      "<Parameter id=\"0\" datatype=\"float\" direction=\"out\"/>"
    "</Parameters>"

    "<Audio>"
      "<Pin id=\"0\" name=\"Value In\" direction=\"in\" datatype=\"float\"  />"
      "<Pin id=\"1\" name=\"PM Value Out\" direction=\"out\" datatype=\"float\" parameterId=\"0\" private =\"true\" autoConfigureParameter=\"true\"/>"
    "</Audio>"

    "<GUI>"
      "<Pin id=\"0\" name=\"PM Value In\" direction=\"in\" datatype=\"float\" parameterId=\"0\" />"
      "<Pin id=\"1\" name=\"PM Name In\" direction=\"in\" datatype=\"string\" parameterId=\"0\" parameterField=\"ShortName\"/>"
      "<Pin id=\"2\" name=\"PM Menu Items\" direction=\"in\" datatype=\"string\" parameterId=\"0\" parameterField=\"MenuItems\"/>"
      "<!-- Little unusual, this is an 'in' even though it's purpose is to send to host-->"
      "<Pin id=\"3\" name=\"PM Menu Selection\" direction=\"in\" datatype=\"int\" parameterId=\"0\" parameterField=\"MenuSelection\"/>"
      "<Pin id=\"4\" name=\"AnimationIn\" direction=\"in\" datatype=\"float\" parameterId=\"0\" parameterField=\"Normalized\"/>"

      "<Pin id=\"5\" name=\"Name\" direction=\"out\" datatype=\"string\" />"
      "<Pin id=\"6\" name=\"Value\" direction=\"out\" datatype=\"float\" />"
      "<!-- legacy only. for replacing the Patch Mem Float Out - B -->"
      "<Pin id=\"7\" name=\"Value Out\" direction=\"in\" datatype=\"float\"/>"
      "<Pin id=\"8\" name=\"Animation Position\" direction=\"out\" datatype=\"float\" />"
      "<Pin id=\"9\" name=\"Menu Items\" direction=\"out\" datatype=\"string\"/>"
      "<Pin id=\"10\" name=\"Menu Selection\" direction=\"out\" datatype=\"int\"/>"
    "</GUI>"
  "</Plugin>"

"</PluginList>"
;
