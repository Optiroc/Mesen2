using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using Mesen.GUI.Config;
using Mesen.Localization;

namespace Mesen.Controls
{
	public class EnumComboBox : UserControl
	{
		public static readonly StyledProperty<Enum> SelectedItemProperty = AvaloniaProperty.Register<EnumComboBox, Enum>(nameof(SelectedItem), null, false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<Type> EnumTypeProperty = AvaloniaProperty.Register<EnumComboBox, Type>(nameof(EnumType));

		public Enum SelectedItem
		{
			get { return GetValue(SelectedItemProperty); }
			set { SetValue(SelectedItemProperty, value); }
		}

		public Type EnumType
		{
			get { return GetValue(EnumTypeProperty); }
			set { SetValue(EnumTypeProperty, value); }
		}

		public EnumComboBox()
		{
			InitializeComponent();
		}

		protected override void OnInitialized()
		{
			base.OnInitialized();

			if(Design.IsDesignMode && this.EnumType == null) {
				return;
			}

			this.GetPropertyChangedObservable(SelectedItemProperty).Subscribe(_ => _.ToString());

			ComboBox cbo = this.FindControl<ComboBox>("ComboBox");

			List<string> values = new List<string>();
			foreach(Enum val in Enum.GetValues(this.EnumType)) {
				values.Add(ResourceHelper.GetEnumText(val));
			}

			cbo.Items = values;
			cbo.SelectedItem = ResourceHelper.GetEnumText(this.SelectedItem);
			cbo.SelectionChanged += MesenComboBox_SelectionChanged;
		}

		private void MesenComboBox_SelectionChanged(object? sender, SelectionChangedEventArgs e)
		{
			if(sender == null) {
				return;
			}

			ComboBox cbo = (ComboBox)sender;
			object? selectedItem = cbo.SelectedItem;
			if(selectedItem is string) {
				foreach(Enum val in Enum.GetValues(this.EnumType)) {
					if((selectedItem as string) == ResourceHelper.GetEnumText(val)) {
						this.SelectedItem = val;
					}
				}
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}