
# X509 library
include_directories(impl)
add_library(ocpp_x509 OBJECT
    Base64.cpp
    Certificate.cpp
    CertificateRequest.cpp
    PrivateKey.cpp
    Sha2.cpp
    X509Document.cpp

    impl/sign.cpp
 )
target_include_directories(ocpp_x509 PUBLIC .)
target_link_libraries(ocpp_x509 PUBLIC ocpp_helpers OpenSSL::SSL OpenSSL::Crypto)
