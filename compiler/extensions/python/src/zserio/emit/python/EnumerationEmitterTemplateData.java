package zserio.emit.python;

import java.util.ArrayList;
import java.util.List;

import zserio.ast.BitFieldType;
import zserio.ast.EnumItem;
import zserio.ast.EnumType;
import zserio.ast.IntegerType;
import zserio.ast.StdIntegerType;
import zserio.emit.common.ExpressionFormatter;
import zserio.emit.common.ZserioEmitException;

public class EnumerationEmitterTemplateData extends UserTypeTemplateData
{
    public EnumerationEmitterTemplateData(TemplateDataContext context, EnumType enumType)
            throws ZserioEmitException
    {
        super(context, enumType);

        final IntegerType enumBaseType = enumType.getIntegerBaseType();
        bitSize = createBitSize(enumBaseType);
        if (bitSize == null)
            importPackage("zserio");

        final ExpressionFormatter pythonExpressionFormatter = context.getPythonExpressionFormatter(this);
        runtimeFunction = PythonRuntimeFunctionDataCreator.createData(enumBaseType, pythonExpressionFormatter);

        final List<EnumItem> enumItems = enumType.getItems();
        items = new ArrayList<EnumItemData>(enumItems.size());
        for (EnumItem enumItem : enumItems)
            items.add(new EnumItemData(enumItem));
    }

    public String getBitSize()
    {
        return bitSize;
    }

    public RuntimeFunctionTemplateData getRuntimeFunction()
    {
        return runtimeFunction;
    }

    public Iterable<EnumItemData> getItems()
    {
        return items;
    }

    public static class EnumItemData
    {
        public EnumItemData(EnumItem enumItem) throws ZserioEmitException
        {
            name = enumItem.getName();
            value = PythonLiteralFormatter.formatDecimalLiteral(enumItem.getValue());
        }

        public String getName()
        {
            return name;
        }

        public String getValue()
        {
            return value;
        }

        private final String name;
        private final String value;
    }

    private static String createBitSize(IntegerType enumBaseType)
    {
        if (enumBaseType instanceof StdIntegerType)
            return PythonLiteralFormatter.formatDecimalLiteral(((StdIntegerType)enumBaseType).getBitSize());
        else if (enumBaseType instanceof BitFieldType)
            return PythonLiteralFormatter.formatDecimalLiteral(((BitFieldType)enumBaseType).getBitSize());
        else
            return null;
    }

    private final String bitSize;
    private final RuntimeFunctionTemplateData runtimeFunction;
    private final List<EnumItemData> items;
}
