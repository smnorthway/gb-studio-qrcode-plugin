const id = "SMN_EVENT_QR_DISPLAY";
const groups = ["EVENT_GROUP_DIALOGUE"];
const name = "Display QR Code";

const fields = [
  {
    key: "text",
    type: "textarea",
    placeholder: "https://example.com/?player=%c$nm1%c$nm2%c$nm3&score=$score",
    defaultValue: "",
    flexBasis: "100%",
  }
];

const compile = (config, helpers) => {
  const { _loadStructuredText, appendRaw } = helpers;
  
  // 1. Load the text with variable expansion
  _loadStructuredText(config.text);
  
  // 2. Call the native function using raw VM instruction
  // We use the symbols confirmed in the project's .gbsres files
  appendRaw("VM_CALL_NATIVE b_hello_world_c _hello_world_c");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  helper: {
    type: "textdraw",
    text: "text",
  },
};
