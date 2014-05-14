

std::map<std::string, Variant> map_in;
map_in["bool"] = TypedVariant<bool>(true);
map_in["byte"] = TypedVariant<std::uint8_t>(7);
map_in["str"]  = TypedVariant<std::string>("Hello World!");
auto in = TypedVariant<std::map<std::string, Variant>>(map_in);

auto out = Variant::fromGVariant(g_variant_ref(in));
auto map_out = out.as<std::map<std::string, Variant>>();

std::cout << "bool: " << map_out["bool"].as<bool>() << std::endl;
std::cout << "byte: " << (std::uint32_t)(map_out["byte"].as<std::uint8_t>()) << std::endl;
std::cout << "str: "  << map_out["str"].as<std::string>() << std::endl;
