package zserio.emit.python;

import zserio.ast.ConstType;
import zserio.ast.EnumItem;
import zserio.ast.EnumType;
import zserio.ast.Expression;
import zserio.ast.Field;
import zserio.ast.Package;
import zserio.ast.Parameter;
import zserio.ast.ZserioType;
import zserio.emit.common.DefaultExpressionFormattingPolicy;
import zserio.emit.common.ZserioEmitException;
import zserio.emit.python.types.PythonNativeType;

public class PythonExpressionFormattingPolicy extends DefaultExpressionFormattingPolicy
{
    public PythonExpressionFormattingPolicy(PythonNativeTypeMapper pythonNativeTypeMapper,
            ImportCollector importCollector)
    {
        this.pythonNativeTypeMapper = pythonNativeTypeMapper;
        this.importCollector = importCollector;
    }

    @Override
    public String getDecimalLiteral(Expression expr, boolean isNegative)
    {
        // decimal literals in Python are the same
        return expr.getText();
    }

    @Override
    public String getBinaryLiteral(Expression expr, boolean isNegative)
    {
        // binary literals in Python are the same (with prefix "0b")
        return PYTHON_BINARY_LITERAL_PREFIX + expr.getText();
    }

    @Override
    public String getHexadecimalLiteral(Expression expr, boolean isNegative)
    {
        // hexadecimal literals in Python are the same (with prefix "0x")
        return PYTHON_HEXADECIMAL_LITERAL_PREFIX + expr.getText();
    }

    @Override
    public String getOctalLiteral(Expression expr, boolean isNegative)
    {
        // octal literals in Python are the same (with prefix "0o")
        return PYTHON_OCTAL_LITERAL_PREFIX + expr.getText();
    }

    @Override
    public String getFloatLiteral(Expression expr, boolean isNegative)
    {
        // Python doesn't have float literals, use double
        return getDoubleLiteral(expr, isNegative);
    }

    @Override
    public String getDoubleLiteral(Expression expr, boolean isNegative)
    {
        // double literals in Python are the same (no suffix)
        return expr.getText();
    }

    @Override
    public String getBoolLiteral(Expression expr)
    {
        final String boolLiteral = expr.getText();
        return PythonLiteralFormatter.formatBooleanLiteral(boolLiteral.equals("true"));
    }

    @Override
    public String getStringLiteral(Expression expr)
    {
        // string literals supports both unicode ('\u0000') and hexadecimal ('\x42') escapes
        return expr.getText();
    }

    @Override
    public String getIndex(Expression expr)
    {
        return "_index";
    }

    @Override
    public String getIdentifier(Expression expr, boolean isLast, boolean isSetter) throws ZserioEmitException
    {
        final StringBuilder result = new StringBuilder();
        final String symbol = expr.getText();
        final Object resolvedSymbol = expr.getExprSymbolObject();
        if (resolvedSymbol instanceof ZserioType)
        {
            // package identifiers are part of this Zserio type
            formatIdentifierForType(result, symbol, (ZserioType)resolvedSymbol);
        }
        else if (!(resolvedSymbol instanceof Package))
        {
            final boolean isFirst = (expr.getExprZserioType() != null); // first in a dot expression
            formatIdentifierForSymbol(result, symbol, isFirst, resolvedSymbol, isSetter);
        }

        return result.toString();
    }

    private void formatIdentifierForType(StringBuilder result, String symbol,
            ZserioType resolvedType) throws ZserioEmitException
    {
        if (resolvedType instanceof EnumType)
        {
            // [EnumType].ENUM_ITEM
            final EnumType enumType = (EnumType)resolvedType;
            final PythonNativeType nativeEnumType = pythonNativeTypeMapper.getPythonType(enumType);
            result.append(nativeEnumType.getFullName());
            importCollector.importType(nativeEnumType);
        }
        else if (resolvedType instanceof ConstType)
        {
            // [ConstName]
            final ConstType constType = (ConstType)resolvedType;
            final PythonNativeType nativeConstType = pythonNativeTypeMapper.getPythonType(constType);
            result.append(nativeConstType.getFullName());
            importCollector.importType(nativeConstType);
        }
        else
        {
            // [functionCall]()
            result.append(symbol);
        }
    }

    private void formatIdentifierForSymbol(StringBuilder result, String symbol, boolean isFirst,
            Object resolvedSymbol, boolean isSetter) throws ZserioEmitException
    {
        if (resolvedSymbol instanceof Parameter)
        {
            final Parameter param = (Parameter)resolvedSymbol;
            formatParameterAccessor(result, isFirst, param, isSetter);
        }
        else if (resolvedSymbol instanceof Field)
        {
            final Field field = (Field)resolvedSymbol;
            formatFieldAccessor(result, isFirst, field, isSetter);
        }
        else if (resolvedSymbol instanceof EnumItem)
        {
            // EnumType.[ENUM_ITEM]
            // emit the whole name if this is the first symbol in this dot subtree, otherwise emit only the
            // enum short name
            final EnumItem item = (EnumItem)resolvedSymbol;
            if (isFirst)
            {
                final EnumType enumType = item.getEnumType();
                final PythonNativeType nativeEnumType = pythonNativeTypeMapper.getPythonType(enumType);
                result.append(nativeEnumType.getFullName());
                result.append(".");
            }
            result.append(item.getName());
        }
    }

    private void formatParameterAccessor(StringBuilder result, boolean isFirst, Parameter param,
            boolean isSetter)
    {
        if (isFirst)
            result.append(getAccessPrefix());

        if (isSetter)
        {
            result.append(AccessorNameFormatter.getSetterName(param));
            result.append(PYTHON_SETTER_FUNCTION_CALL);
        }
        else
        {
            result.append(AccessorNameFormatter.getGetterName(param));
            result.append(PYTHON_GETTER_FUNCTION_CALL);
        }
    }

    private void formatFieldAccessor(StringBuilder result, boolean isFirst, Field field, boolean isSetter)
    {
        if (isFirst)
            result.append(getAccessPrefix());

        if (isSetter)
        {
            result.append(AccessorNameFormatter.getSetterName(field));
            result.append(PYTHON_SETTER_FUNCTION_CALL);
        }
        else
        {
            result.append(AccessorNameFormatter.getGetterName(field));
            result.append(PYTHON_GETTER_FUNCTION_CALL);
        }
    }

    private String getAccessPrefix()
    {
        // TODO:
        return "";
    }

    @Override
    public UnaryExpressionFormatting getFunctionCall(Expression expr)
    {
        return new UnaryExpressionFormatting("", "()");
    }

    @Override
    public UnaryExpressionFormatting getLengthOf(Expression expr)
    {
        return new UnaryExpressionFormatting("len(", ")");
    }

    @Override
    public UnaryExpressionFormatting getSum(Expression expr)
    {
        return new UnaryExpressionFormatting("sum(", ")");
    }

    @Override
    public UnaryExpressionFormatting getExplicit(Expression expr)
    {
        return new UnaryExpressionFormatting("");
    }

    @Override
    public UnaryExpressionFormatting getNumBits(Expression expr)
    {
        importCollector.importRuntime();
        return new UnaryExpressionFormatting("zserio.builtin.getNumBits(", ")");
    }

    @Override
    public BinaryExpressionFormatting getArrayElement(Expression expr, boolean isSetter)
    {
        return new BinaryExpressionFormatting("", "[", (isSetter) ? "] = _value" : "]");
    }

    @Override
    public BinaryExpressionFormatting getDot(Expression expr)
    {
        // ignore dots between package identifiers
        if (expr.op1().getExprZserioType() == null)
            return new BinaryExpressionFormatting("");

        return new BinaryExpressionFormatting(".");
    }

    @Override
    public TernaryExpressionFormatting getQuestionMark(Expression expr)
    {
        return new TernaryExpressionFormattingPython(expr, "(", ") if (", ") else (", ")");
    }

    private static class TernaryExpressionFormattingPython extends TernaryExpressionFormatting
    {
        public TernaryExpressionFormattingPython(Expression expression, String beforeOperand1, String afterOperand1,
                String afterOperand2, String afterOperand3)
        {
            super(expression, beforeOperand1, afterOperand1, afterOperand2, afterOperand3);
        }

        @Override
        public Expression getOperand1()
        {
            return super.getOperand2();
        }

        @Override
        public Expression getOperand2()
        {
            return super.getOperand1();
        }
    }

    private final PythonNativeTypeMapper pythonNativeTypeMapper;
    private final ImportCollector importCollector;

    private final static String PYTHON_BINARY_LITERAL_PREFIX = "0b";
    private final static String PYTHON_HEXADECIMAL_LITERAL_PREFIX = "0x";
    private final static String PYTHON_OCTAL_LITERAL_PREFIX = "0o";

    private final static String PYTHON_GETTER_FUNCTION_CALL = "()";
    private final static String PYTHON_SETTER_FUNCTION_CALL = "(_value)";
}