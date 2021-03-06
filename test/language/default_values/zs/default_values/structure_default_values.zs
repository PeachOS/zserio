package default_values.structure_default_values;

enum uint8 BasicColor
{
    BLACK,
    WHITE,
    RED
};

subtype bit:4 Nibble;

struct StructureDefaultValues
{
    bool        boolValue = true;
    Nibble      bit4Value = 0x0F if boolValue == true;
    int16       int16Value = 0x0BEE;
    float16     float16Value = 1.23f;
    float32     float32Value = 1.234f;
    float64     float64Value = 1.2345;
    string      stringValue = "string";
    BasicColor  enumValue = BasicColor.BLACK;
};
