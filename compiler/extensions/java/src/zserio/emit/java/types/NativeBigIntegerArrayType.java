package zserio.emit.java.types;

public class NativeBigIntegerArrayType extends NativeArrayType
{
    public NativeBigIntegerArrayType()
    {
        super("BigIntegerArray");
    }

    @Override
    public boolean requiresElementBitSize()
    {
        return true;
    }
}
