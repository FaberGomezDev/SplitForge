using SplitForge.Core.Input;

namespace SplitForge.Tests;

public class InputTests
{
    [Fact]
    public void DeadzoneAndClamping()
    {
        Assert.Equal(0f, AxisMath.Normalize(0.10f));
        Assert.Equal(1f, AxisMath.Normalize(1.2f));
        Assert.Equal(-1f, AxisMath.Normalize(-1.2f));
        Assert.InRange(AxisMath.Normalize(0.6f), 0.01f, 0.99f);
        Assert.Equal(-1f, AxisMath.FromSdl(short.MinValue));
        Assert.Equal(1f, AxisMath.FromSdl(short.MaxValue));
    }

    [Fact]
    public void EdgeDetectionIgnoresHeldButtons()
    {
        Assert.Empty(AxisMath.NewPresses([3], [3]));
        Assert.Equal([4], AxisMath.NewPresses([4], []));
        Assert.Equal([4], AxisMath.NewPresses([3, 4], [3]));
    }

    [Fact]
    public void MappingValidation()
    {
        var m = new ControllerMapping { AxisX = 2, AxisY = 3, FireButton = 4, InvertY = true }.Validate(6, 12);
        Assert.Equal((2, 3, 4), (m.AxisX, m.AxisY, m.FireButton));
        Assert.Throws<ArgumentException>(() => new ControllerMapping { AxisY = 9 }.Validate(3, 6));
        Assert.Throws<ArgumentException>(() => new ControllerMapping { FireButton = -1 }.Validate(3, 6));
        Assert.Throws<ArgumentException>(() => new ControllerMapping { FireAxis = 2, FireDirection = 0 }.Validate(3, 6));
    }

    [Fact]
    public void OptionalActionsForOneButtonDevice()
    {
        var m = ControllerMapping.DefaultFor(1).Validate(2, 1);
        Assert.Null(m.DashButton);
        Assert.Null(m.ShieldButton);
        Assert.Throws<ArgumentException>(() => new ControllerMapping { FireButton = 8 }.Validate(2, 1));
    }

    [Fact]
    public void Ps4FireCanBeNonZeroIndexAndSeparateFromDashShield()
    {
        var mapping = new ControllerMapping { FireButton = 5, DashButton = 1, ShieldButton = 2 };
        bool[] buttons = [false, true, false, false, false, true, false, false];
        var s = PadInterpreter.FromJoystick(101, [0.2f, 0f], buttons, mapping);
        Assert.True(s.Fire);
        Assert.True(s.Dash);
        Assert.False(s.Shield);
        buttons[5] = false;
        buttons[2] = true;
        var after = PadInterpreter.FromJoystick(101, [0.2f, 0f], buttons, mapping);
        Assert.False(after.Fire);
        Assert.True(after.Shield);
    }

    [Fact]
    public void AnalogR2ReplacesDigitalFire()
    {
        // R2 en reposo a −1 (SDL) y pulsado hacia +1.
        var mapping = new ControllerMapping { FireButton = 0, FireAxis = 5, FireRest = -1f, FireDirection = 1 };
        float[] axes = [0, 0, 0, 0, -1, -1];
        bool[] buttons = [true, false, false];
        Assert.False(PadInterpreter.FromJoystick(1, axes, buttons, mapping).Fire);
        axes[5] = 0.2f;
        Assert.True(PadInterpreter.FromJoystick(1, axes, buttons, mapping).Fire);
    }

    [Fact]
    public void JoystickYIsUpPositiveAndInvertible()
    {
        var up = PadInterpreter.FromJoystick(1, [0.75f, -0.9f], [true], ControllerMapping.DefaultFor(1));
        Assert.True(up.LX > 0.65f);
        Assert.True(up.LY > 0.8f);
        var inverted = PadInterpreter.FromJoystick(1, [0f, 0f, -0.8f, 0.6f], [false, false, true],
            new ControllerMapping { AxisX = 2, AxisY = 3, FireButton = 2, InvertY = true });
        Assert.True(inverted.Fire);
        Assert.True(inverted.LX < 0);
        Assert.True(inverted.LY > 0);
    }

    [Fact]
    public void GamepadPath()
    {
        var s = PadInterpreter.FromGamepad(7, 0.5f, -1f, true, false, true);
        Assert.True(s.LY > 0.98f);
        Assert.Equal(0.4186f, s.LX, 2);
        Assert.True(s.Fire);
        Assert.True(s.Shield);
        Assert.False(s.Dash);
    }

    [Fact]
    public void ReadPathPolicy_()
    {
        var ps4 = new PadInfo(1, "PS4 Controller", "G", PadKind.StandardGamepad, 6, 16);
        var ds4 = new PadInfo(3, "Wireless Controller", "G", PadKind.GenericHid, 6, 14);
        var xb = new PadInfo(2, "Xbox One Controller", "G", PadKind.StandardGamepad, 6, 12);
        Assert.Equal(ReadPath.Joystick, ReadPathPolicy.Choose(ps4, true, InputReadMode.Auto));
        Assert.Equal(ReadPath.Joystick, ReadPathPolicy.Choose(ds4, true, InputReadMode.Auto));
        Assert.Equal(ReadPath.Gamepad, ReadPathPolicy.Choose(xb, true, InputReadMode.Auto));
        Assert.Equal(ReadPath.Joystick, ReadPathPolicy.Choose(xb, true, InputReadMode.Joystick));
        Assert.Equal(ReadPath.Joystick, ReadPathPolicy.Choose(ps4, false, InputReadMode.Gamepad));
        Assert.Equal(ReadPath.Gamepad, ReadPathPolicy.Choose(ps4, true, InputReadMode.Gamepad));
        Assert.Equal(InputReadMode.Gamepad, ReadPathPolicy.ParseLegacy("controller"));
    }

    [Fact]
    public void TwoPs4SameModelStayIndependent()
    {
        var m = ControllerMapping.DefaultFor(1);
        var a = PadInterpreter.FromJoystick(101, [0.75f, -0.8f], [true], m);
        var b = PadInterpreter.FromJoystick(102, [-0.6f, 0.4f], [false], m);
        Assert.True(a.LX > 0.6f && a.Fire);
        Assert.True(b.LX < -0.5f && !b.Fire);
        Assert.NotEqual(a.DeviceId, b.DeviceId);
    }
}
