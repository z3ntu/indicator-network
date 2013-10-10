using GLib;

namespace Utils {

  public bool variant_contains (Variant variant, string needle)
  {
    if (variant.is_of_type(VariantType.VARIANT))
      return variant_contains(variant.get_variant(), needle);

    if (!variant.is_container())
      return false;

    Variant item;
    var iter = new VariantIter(variant);
    for (item = iter.next_value(); item != null; item = iter.next_value()) {
      if (item.get_string() == needle)
        return true;
    }

    return false;
  }

}
