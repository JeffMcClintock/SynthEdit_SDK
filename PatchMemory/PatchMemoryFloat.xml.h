// File genereated by XmlToHeaderFile.exe from module xml definition.
const char* PATCHMEMORYFLOAT_XML = 
"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"

"<PluginList>"
  "<Plugin id=\"SE PatchMemory Float\"  name=\"PatchMemory Float3\" category=\"Sub-Controls\" helpUrl=\"Patchmemory.htm\" >"
    "<Parameters>"
      "<Parameter id=\"0\" datatype=\"float\" />"
    "</Parameters>"

    "<Audio>"
      "<Pin id=\"0\" name=\"Value In\" direction=\"in\" datatype=\"float\" parameterId=\"0\" />"
      "<Pin id=\"1\" name=\"Value Out\" direction=\"out\" datatype=\"float\" autoConfigureParameter=\"true\"/>"
    "</Audio>"

    "<GUI>"
      "<Pin id=\"0\" name=\"Value In\" direction=\"in\" datatype=\"float\" parameterId=\"0\" />"
      "<Pin id=\"1\" name=\"Name In\" direction=\"in\" datatype=\"string\" parameterId=\"0\" parameterField=\"ShortName\"/>"
      "<Pin id=\"2\" name=\"Menu Items\" direction=\"in\" datatype=\"string\" parameterId=\"0\" parameterField=\"MenuItems\"/>"
      "<!-- Little unusual, this is an 'in' even though it's purpose is to send to host-->"
      "<Pin id=\"3\" name=\"Menu Selection\" direction=\"in\" datatype=\"int\" parameterId=\"0\" parameterField=\"MenuSelection\"/>"
      "<Pin id=\"4\" name=\"Mouse Down\" direction=\"in\" datatype=\"bool\" parameterId=\"0\" parameterField=\"Grab\"/>"
      "<Pin id=\"5\" name=\"AnimationIn\" direction=\"in\" datatype=\"float\" parameterId=\"0\" parameterField=\"Normalized\"/>"

      "<Pin id=\"6\" name=\"Name\" direction=\"out\" datatype=\"string\" />"
      "<Pin id=\"7\" name=\"Value\" direction=\"out\" datatype=\"float\" />"
      "<Pin id=\"8\" name=\"Animation Position\" direction=\"out\" datatype=\"float\" />"
      "<Pin id=\"9\" name=\"Menu Items\" direction=\"out\" datatype=\"string\"/>"
      "<Pin id=\"10\" name=\"Menu Selection\" direction=\"out\" datatype=\"int\"/>"
      "<Pin id=\"11\" name=\"Mouse Down\" direction=\"out\" datatype=\"bool\"/>"
    "</GUI>"
  "</Plugin>"
"</PluginList>"
;
