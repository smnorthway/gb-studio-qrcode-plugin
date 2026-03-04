const id = "SMN_EVENT_QR_HASH";
const groups = ["EVENT_GROUP_DIALOGUE"];
const name = "Calculate QR Hash";

const fields = [
  {
    key: "text",
    type: "textarea",
    label: "Hash Source String",
    placeholder: "hello#world",
    defaultValue: "",
    flexBasis: "100%",
  },
  {
    key: "var1",
    label: "Result Variable 1",
    type: "variable",
    defaultValue: "LAST_VARIABLE",
  },
  {
    key: "var2",
    label: "Result Variable 2",
    type: "variable",
    defaultValue: "LAST_VARIABLE",
  },
  {
    key: "var3",
    label: "Result Variable 3",
    type: "variable",
    defaultValue: "LAST_VARIABLE",
  },
  {
    key: "var4",
    label: "Result Variable 4",
    type: "variable",
    defaultValue: "LAST_VARIABLE",
  },
  {
    key: "var5",
    label: "Result Variable 5",
    type: "variable",
    defaultValue: "LAST_VARIABLE",
  }
];

const compile = (config, helpers) => {
  const { _loadStructuredText, appendRaw, getVariableAlias } = helpers;
  
  // 1. Load the text with variable expansion into VM buffer
  _loadStructuredText(config.text);

  // 2. Push variable indices onto stack in reverse order
  // (so pop order in C is 1, 2, 3, 4, 5)
  // Actually, we'll pop them in the same order we push if we use standard pop, 
  // but usually it's LIFO. Let's push 5, 4, 3, 2, 1.
  appendRaw(`VM_PUSH_CONST ${getVariableAlias(config.var5)}`);
  appendRaw(`VM_PUSH_CONST ${getVariableAlias(config.var4)}`);
  appendRaw(`VM_PUSH_CONST ${getVariableAlias(config.var3)}`);
  appendRaw(`VM_PUSH_CONST ${getVariableAlias(config.var2)}`);
  appendRaw(`VM_PUSH_CONST ${getVariableAlias(config.var1)}`);
  
  // 3. Call the native hash function
  appendRaw("VM_CALL_NATIVE b_calculate_hash_c _calculate_hash_c");
  
  // 4. Cleanup stack (pop 5 values)
  appendRaw("VM_POP 5");
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
