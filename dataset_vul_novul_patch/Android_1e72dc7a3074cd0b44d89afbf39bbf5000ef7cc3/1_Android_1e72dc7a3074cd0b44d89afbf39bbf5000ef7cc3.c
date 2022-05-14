static jlong Region_createFromParcel(JNIEnv* env, jobject clazz, jobject parcel)
{
 if (parcel == NULL) {
 return NULL;
 }

    android::Parcel* p = android::parcelForJavaObject(env, parcel);

 const size_t size = p->readInt32();
 const void* regionData = p->readInplace(size);
 if (regionData == NULL) {

         return NULL;
     }
     SkRegion* region = new SkRegion;
    region->readFromMemory(regionData, size);
 
     return reinterpret_cast<jlong>(region);
 }
